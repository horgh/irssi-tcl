#  idna.tcl --
#  
#      This file is part of the jabberlib. It provides support for
#      Internationalizing Domain Names in Applications (IDNA, RFC 3490).
#      
#  Copyright (c) 2005 Alexey Shchepin <alexey@sevcom.net>
#  
# $Id$
#
#  SYNOPSIS
#      idna::domain_toascii domain
#

#
# Found at http://svn.xmpp.ru/repos/tkabber/trunk/tkabber/idna.tcl
#

##########################################################################

package provide idna 1.0

##########################################################################

namespace eval idna {}

##########################################################################

proc idna::domain_toascii {domain} {
    set domain [string tolower $domain]
    set parts [split $domain "\u002E\u3002\uFF0E\uFF61"]
    set res {}
    foreach p $parts {
	set r [toascii $p]
	lappend res $r
    }
    return [join $res .]
}

##########################################################################

proc idna::toascii {name} {
    # TODO: Steps 2, 3 and 5 from RFC3490

    if {![string is ascii $name]} {
	set name [punycode_encode $name]
	set name "xn--$name"
    }
    return $name
}

##########################################################################

proc idna::punycode_encode {input} {
    set base 36
    set tmin 1
    set tmax 26
    set skew 38
    set damp 700
    set initial_bias 72
    set initial_n 0x80

    set n $initial_n
    set delta 0
    set out 0
    set bias $initial_bias
    set output ""
    set input_length [string length $input]
    set nonbasic {}

    for {set j 0} {$j < $input_length} {incr j} {
	set c [string index $input $j]
	if {[string is ascii $c]} {
	    append output $c
	} else {
	    lappend nonbasic $c
	}
    }

    set nonbasic [lsort -unique $nonbasic]

    set h [set b [string length $output]];

    if {$b > 0} {
	append output -
    }

    while {$h < $input_length} {
	set m [scan [string index $nonbasic 0] %c]
	set nonbasic [lrange $nonbasic 1 end]

	incr delta [expr {($m - $n) * ($h + 1)}]
	set n $m

	for {set j 0} {$j < $input_length} {incr j} {
	    set c [scan [string index $input $j] %c]

	    if {$c < $n} {
		incr delta
	    } elseif {$c == $n} {
		for {set q $delta; set k $base} {1} {incr k $base} {
		    set t [expr {$k <= $bias ? $tmin :
				 $k >= $bias + $tmax ? $tmax : $k - $bias}]
		    if {$q < $t} break;
		    append output \
			[punycode_encode_digit \
			     [expr {$t + ($q - $t) % ($base - $t)}]]
		    set q [expr {($q - $t) / ($base - $t)}]
		}

		append output [punycode_encode_digit $q]
		set bias [punycode_adapt \
			      $delta [expr {$h + 1}] [expr {$h == $b}]]
		set delta 0
		incr h
	    }
	}
	
	incr delta
	incr n
    }

    return $output;
}

##########################################################################

proc idna::punycode_adapt {delta numpoints firsttime} {
    set base 36
    set tmin 1
    set tmax 26
    set skew 38
    set damp 700

    set delta [expr {$firsttime ? $delta / $damp : $delta >> 1}]
    incr delta [expr {$delta / $numpoints}]

    for {set k 0} {$delta > (($base - $tmin) * $tmax) / 2}  {incr k $base} {
	set delta [expr {$delta / ($base - $tmin)}];
    }

    return [expr {$k + ($base - $tmin + 1) * $delta / ($delta + $skew)}]
}

##########################################################################

proc idna::punycode_encode_digit {d} {
    return [format %c [expr {$d + 22 + 75 * ($d < 26)}]]
}

##########################################################################


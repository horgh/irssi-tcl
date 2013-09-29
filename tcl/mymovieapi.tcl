#
# 2013-09-28
# will@summercat.com
#
# search using the mymovieapi.com api
#

package require http
package require json

namespace eval ::mma {
	# string
	variable baseurl {http://mymovieapi.com/}

	# dict. cache of results.
	variable cache {}

	# int. http timeout.
	variable http_timeout 60000

	# bool. toggle debug output.
	variable debug 1

	signal_add msg_pub .imdb ::mma::pub_handler
	signal_add msg_pub !imdb ::mma::pub_handler

	settings_add_str "mymovieapi_enabled_channels" ""
}

proc ::mma::::log {msg} {
	if {!$::mma::debug} {
		return
	}
	irssi_print "mymovieapi: $msg"
}

# output a single search result from a search query.
# data is a dict - converted json result.
proc ::mma::output_search_result {server chan data} {
	::mma::log "output_search_result: data: $data"

	# dict key we want.
	# we want to check that each is present and have a sane default
	# if it is not.
	set fields [list title type year rating rating_count plot_simple imdb_url country genres]
	# dict with keys/values we can count on existing and which
	# we will populate from the result.
	set d [dict create]
	foreach field $fields {
		if {![dict exists $data $field]} {
			::mma::log "output_search_result: field $field not found"
			dict append d $field {}
			continue
		}
		set value [dict get $data $field]
		::mma::log "output_search_result: field $field value $value"
		dict append d $field $value
	}
	set title        [dict get $d title]
	set type         [dict get $d type]
	set year         [dict get $d year]
	set rating       [dict get $d rating]
	set rating_count [dict get $d rating_count]
	set plot_simple  [dict get $d plot_simple]
	set imdb_url     [dict get $d imdb_url]
	set countries    [dict get $d country]
	set genres       [dict get $d genres]

	set genre_str ""
	foreach genre $genres {
		append genre_str "$genre "
	}
	set genre_str [string trim $genre_str]

	set country_str ""
	foreach country $countries {
		append country_str "$country "
	}
	set country_str [string trim $country_str]

	set o "\002$title\002 ($year) ($type) ($country_str) Rating: $rating ($rating_count votes) ($genre_str) $plot_simple"
	::mma::log "output_search_result: output string: $o"
	putchan $server $chan $o
}

# output the response from a search query.
# data is a list of dicts - the converted json.
# TODO confirm this is a list
proc ::mma::output_search {server chan data} {
	::mma::log "output_search: in output_search"

	# we may have an error (such as no result).
	# we receive an error if this is not a dict (such as when
	# we have results) so wrap it in a catch.
	if {![catch {dict exists $data error} exists]} {
		# error exists.
		set error [dict get $data error]
		putchan $server $chan "Error: $error"
		return
	}

	foreach result $data {
		::mma::output_search_result $server $chan $result
	}
}

# callback from http::geturl for a search query.
proc ::mma::search_cb {server chan token} {
	::mma::log "search_cb: in callback"
	set data [::http::data $token]
	::mma::log "search_db: data: $data"
	::http::cleanup $token

	# json2dict can return success if we receive an html page
	# (not sure why - some garbage input causes errors to be raised)
	# where the resulting 'dict' is just '7'. so run a quick
	# check if we receive what looks to be html - we receive an html
	# response if the api is down (cloudflare).
	set data [string trim $data]
	if {[string index $data 0] == "<"} {
		::mma::log "search_cb: response looks like html"
		putchan $server $chan "API appears to be down."
		return
	}

	# convert the json response.
	if {[catch {::json::json2dict $data} data]} {
		::mma::log "search_cb: failure converting to json: $data"
	  putchan $server $chan "Failed to parse the response"
		return
	}
	::mma::log "search_cb: json $data"

	# cache it.
	# TODO

	# generate output.
	# wrap in a catch since it is possible for the dict to not
	# be as we expect and generate errors.
	if {[catch {::mma::output_search $server $chan $data} err]} {
		::mma::log "search_cb: failure outputting result(s): $err"
		putchan $server $chan "Failed to generate the output"
		return
	}
}

# perform an api search and output to the server & channel.
# argv is the search parameter.
proc ::mma::search {server chan argv} {
	::mma::log "search: new search \[$argv\]"

	# check cache.
	# TODO

	# q = search term
	# yg = 0 disables year searching. otherwise it defaults to searching
	# within a year (default the current year).
	# limit = 3 => 3 results max.
	set params [list q $argv yg 0 limit 3]
	set query [::http::formatQuery {*}$params]

	#set token [::http::geturl $::mma::baseurl -timeout $::mma::http_timeout \
	#	-command "::mma::search_cb $server $chan" \
	#	-query $query]
	set url $::mma::baseurl
	set url $url?$query
	set token [::http::geturl $url -timeout $::mma::http_timeout \
		-command "::mma::search_cb $server $chan"]
}

# msg_pub signal handler.
# argv is the search parameter.
proc ::mma::pub_handler {server nick uhost chan argv} {
	if {![str_in_settings_str "mymovieapi_enabled_channels" $chan]} {
		return
	}
	set argv [string trim $argv]
	if {$argv == ""} {
		putchan $server $chan "Usage: .imdb <query>"
		return
	}
	::mma::search $server $chan $argv
}

irssi_print "mymovieapi.tcl loaded"

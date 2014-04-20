# Eggdrop scripts

Eggdrop scripts can be converted to run in Irssi with some minor modifications.
Look at the included scripts.
Many of these also have eggdrop versions on my
github in the eggdrop-scripts project.


## a). Modifications for bind functions (e.g. `pub`/`pubm` calls):

  1. Remove the `hand` function argument in bind procs

  2. Add server argument as first function argument in bind proc

         proc someproc {nick uhost hand chan argv} { .. }

     would become

         proc someproc {server nick uhost chan argv} { .. }

  3. Alter all `putserv/`puthelp` calls to have `$server` as their first
     argument

         putserv "PRIVMSG $chan :Hello there"

     would become

         putserv $server "PRIVMSG $chan :Hello there"

    NOTE: putserv will not show output in Irssi.
    It is better to replace putserv
    with `putchan $server $chan "$text"` which will (if a public message...)

  4. Possible gotcha of the above: any calls that are not in the procs that
     are bound directly which use putserv will require an added argument

         proc do_something {chan text} {
             putserv "PRIVMSG $chan :$text"
         }

     would become

         proc do_something {server chan text} {
              putserv $server "PRIVMSG $chan :text"
         }

The script ./utils/eggdrop_convert.tcl can be used for cases 1 - 3 (hopefully)
But it does not deal with putserv to putchan.
See the script for more information.

The script ./utils/eggdrop_convert_putchan.tcl can be used to convert some
cases of putserv to putchan.
See the script for more information.


## b). Change `[channel get $channel value]` to
   `[str_in_settings_str value $channel]`

This is to replace channel `+enable`\'d values for channels with lists
of channels via /set


## c). Change `setudef flag <value>` to instead use
   `settings_add_str "value" ""` or

some other /settings type.
Combine with b) to use.


## d. Change `bind type flag trigger function`

For example, `bind pub -|- !trigger function` to, for instance,
`signal_add msg_pub trigger function`.


## f. For non-blocking scripts you must use callback functions and use Tcl's
   event support.

For the `http` package, this means using the `-command` flag
with `::http::geturl`.

NOTE: errors in the callback do not get passed up to the default
bgerror handler and so do not get printed in Irssi, so it may be
necessary to look at the error and status elements of the state
array to help debug.

See http.tcl `http::Finish()` for where our
callback command is wrapped in `catch {}`.

Another way around this is to wrap your entire callback function in
your own `catch {}` so you have more control over the errors.

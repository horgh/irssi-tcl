# Additional commands available to the Tcl interpreter

## a. Miscellaneous

  * `irssi_cmd`

    Returns full path to ~/.irssi/

  * `irssi_print <text>`

    Prints <text> to the Irssi client with level `MSGLEVEL_CRAP`

  * `signal_add <type> <keyword> <proc_name>`

    Cause signals of given type and keyword to call `proc_name`

  * `load_script <script filename>`

    Loads the script given by the filename into the interpreter. The filename
    must be in ~/.irssi/tcl/

  * `load_script_absolute <script filename>`


## b. Server output

  * `putserv_raw <server_tag> <text>`

    Output raw IRC command to the server identified by `server_tag`

    For example

        putserv_raw "server1" "PRIVMSG #channel :hi there"

    This command does not deal with newlines.
    Newlines will appear to
    indicate a new command to the IRC server.

    Output is not shown on the Irssi client side

  * `putserv <server_tag> <text>`

    This does the same as `putserv_raw` except some cleanup is done to
    the string.
    Newlines and tabs are removed.

  * `putchan_raw <server_tag> <#channel> <text>`

    Output text to #channel on server identified by `server_tag`.

    This command will show output on the Irssi client side.

    Similar to `putserv_raw`, newlines are not handled.

  * `putchan <server_tag> <#channel> <text>`

    This is the same as `putchan_raw` except some cleanup is done to
    the string.
    Newlines and tabs are removed.


## c. Irssi settings

  * `settings_get_str <key>`

    Extracts the Irssi settings string associated with key

  * `settings_add_str <key> <default>`

    Add a key to Irssi settings for a string value with given default

  * `str_in_settings_str <key> <str>`

    Returns true if str is in the settings string for key.
    The key value is assumed to be a list of strings separated by spaces.


## d. signals

  * `emit_message_public <server> <channel> <nick> <address> <text>`

  * `signal_stop`

[**Go to the previous page**](../../README.md)

----

# Debug console

Plugin contains the debug console, which prints different kinds of the debug messages.

----

## How does it look like?

![Screenshot of the "Debug console" appearance](./debug_console_appearance.png)

----

> **Note!**
>
> It is mainly used during the development phase, as allows to see debug messages from implementation right within the plugin.

----

Still, some messages are presented even in the production versions, as:
- messages regarding start-up phase of the settings manager
- messages regarding the time, which was consumed to process the search query
- warning messages regarding the timestamp mismatch within the analyzed file. That impacts possibility to represent average values within the "Grouped view"

----

Besides that, there is an input field, which allows you to get some useful information about the plugin:

![Screenshot of the "Debug console" input field](./debug_console_input.png)

Supported commands are:

help -> get list of supported commands
- clear -> clears console view
- color-aliases -> shows list of the color aliases, which can be used within the regex name scripting
- styles -> represents Qt styles available on target OS. Reference information, which can be used to run dlt-viewer with one of the supported styles.
- support -> represents the support message
- version -> represents the version of the plugin
- web-link -> represents link to plugin's web-site
- uml-id -> represents list of the supported UML types, which is used for cretion of the sequence diagrams

List of the supported commands will be extended in future releases.

----

[**Go to the previous page**](../../README.md)
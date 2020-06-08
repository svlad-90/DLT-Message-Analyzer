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

As of now there is no possibility for the final user of the plugin to send his own messages. Still, we are thinking of the use-cases where that could be applicable.

----

[**Go to the previous page**](../../README.md)
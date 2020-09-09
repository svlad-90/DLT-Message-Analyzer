[**Go to the previous page**](../../README.md)

----

# Advanced highlighting

DLT Message Analyzer plugin supports regex name scripting. It means, that the provided regex name might be considered by the implementation of the plugin and used by it in this or that view.

One of the supported options is the advanced highlighting of the found messages.

----

By default, the plugin will highlight each regex group with some color. Default colors depend on the used user settings. By default, it will be a repetitive gradient consisting of 5 colors:

![Screenshot of gradient coloring syntax](./gradient_syntax.png)

User can exchange the gradient settings in the context menu of the "Search view":

![Screenshot of gradient coloring syntax](./gradient_settings.png)

The default coloring can be overwritten by a regex name script.

Supported syntax options are:

**RGB_R_G_B** => e.g. RGB_0_0_0 stands for black:

![Screenshot of RGB coloring syntax](./rgb_syntax.png)

**Color name** => e.g. BLACK:

![Screenshot of color name syntax](./color_name_syntax.png)

**Status name** => e.g. ERROR:

![Screenshot of status-based coloring syntax](./status_syntax.png)

Supported statuses are:

{"ok", QColor(0,150,0)},<br/>
{"warning", QColor(150,150,0)},<br/>
{"error", QColor(150,0,0)}

Coloring of the nested groups is also supported:

![Screenshot of nested regex groups coloring](./nested_groups_coloring.png)

All the above parameters are case insensitive.

----

# Supported color names

To get the list of the supported color aliases, please, switch to the "Console view" tab, and enter the "color-aliases" command:

![Screenshot of color-aliases command input](./color_aliases_command.png)

![Screenshot of color-aliases command inputresult representation](./color_aliases_result.png)

[**Go to the previous page**](../../README.md)
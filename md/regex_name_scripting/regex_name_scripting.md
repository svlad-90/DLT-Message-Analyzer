[**Go to the previous page**](../../README.md)

----

# Regex name scripting

DLT Message Analyzer plugin supports regex name scripting. It means, that the provided regex name might be considered by the implementation of the plugin and used by it in this or that view.

Currently, the following options are supported.

----

# Text coloring

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

# Variables

When you are working with a complex regex expression, it is quite complex to find an important part of it in order to change some parameters.

E.g. the default regex, which is used in the one of the projects where this plugin is used consists of 1831 characters.
Try to find and change something in such a big string! It is a quite complex task.

To keep things easier, the variable scripting was introduced within the plugin. It does the following thing:

![Screenshot of variables usage](./var_example.png)

As you see above, each "VAR_TRACE_SPAM_APP" regex name will be turned into a separate item in the tree view. It allows to build "regex with parameters" and easily exchange the content of your filter:

![Screenshot of variable edit process](./var_edit.png)

Change in a tree view will be reflected in a final regex.

Also, selection within a tree view will select the corresponding part in the main regex input field:

![Screenshot of variable's selection](./var_selection.png)

If needed it is possible to switch from visualization of "Variables" to visualization of the whole regex:

![Screenshot of turned on variable's filter](./var_filter_variables_on.png)

![Screenshot of turned off variable's filter](./var_filter_variables_off.png)

All the above parameters are case insensitive.

----

> **Note!**
>
> Plugin implementation allows you to search within the variables.
> The search mechanism is based on the regex, thus you can apply some non-trivial search logic:
>
> ![Screenshot of variables search](./var_search.png)
>
> **This functionality is quite useful when you've formed a regex combination, which consists of several thousands characters and includes dozens of variables.**

----

> **Note!**
>
> Be aware, that the used implementation of the regex engine supports groups with a name size of not more than 32 characters.
> If you'll use more characters in the name - engine will interpret the regex as invalid one.

----

# AND operator

If needed, "color" and "variable" scripting options can be combined together with the help of the "_AND_" operator. This operator is case insensitive, so you can use also "_and_", "_AnD_", etc:

![Screenshot of example of the and operator's usage](./var_and_operator.png)

----

# Supported color names

To get the list of the supported color aliases, please, switch to the "Console view" tab, and enter the "color-aliases" command:

![Screenshot of color-aliases command input](./color_aliases_command.png)

![Screenshot of color-aliases command inputresult representation](./color_aliases_result.png)

[**Go to the previous page**](../../README.md)
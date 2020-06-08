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

Finding! Color names split with "space" will currently not work, as regex name can't contain a space. Names will be replaced with "_" instead of " " in the next plugin's release.

{"black", QColor(0,0,0)},<br/>
{"white", QColor(255,255,255)},<br/>
{"red", QColor(255,0,0)},<br/>
{"lime", QColor(0,255,0)},<br/>
{"blue", QColor(0,0,255)},<br/>
{"yellow", QColor(255,255,0)},<br/>
{"cyan", QColor(0,255,255)},<br/>
{"magenta", QColor(255,0,255)},<br/>
{"silver", QColor(192,192,192)},<br/>
{"gray", QColor(128,128,128)},<br/>
{"maroon", QColor(128,0,0)},<br/>
{"olive", QColor(128,128,0)},<br/>
{"green", QColor(0,128,0)},<br/>
{"purple", QColor(128,0,128)},<br/>
{"teal", QColor(0,128,128)},<br/>
{"navy", QColor(0,0,128)},<br/>
{"maroon", QColor(128,0,0)},<br/>
{"dark red", QColor(139,0,0)},<br/>
{"brown", QColor(165,42,42)},<br/>
{"firebrick", QColor(178,34,34)},<br/>
{"crimson", QColor(220,20,60)},<br/>
{"red", QColor(255,0,0)},<br/>
{"tomato", QColor(255,99,71)},<br/>
{"coral", QColor(255,127,80)},<br/>
{"indian red", QColor(205,92,92)},<br/>
{"light coral", QColor(240,128,128)},<br/>
{"dark salmon", QColor(233,150,122)},<br/>
{"salmon", QColor(250,128,114)},<br/>
{"light salmon", QColor(255,160,122)},<br/>
{"orange red", QColor(255,69,0)},<br/>
{"dark orange", QColor(255,140,0)},<br/>
{"orange", QColor(255,165,0)},<br/>
{"gold", QColor(255,215,0)},<br/>
{"dark golden rod", QColor(184,134,11)},<br/>
{"golden rod", QColor(218,165,32)},<br/>
{"pale golden rod", QColor(238,232,170)},<br/>
{"dark khaki", QColor(189,183,107)},<br/>
{"khaki", QColor(240,230,140)},<br/>
{"olive", QColor(128,128,0)},<br/>
{"yellow", QColor(255,255,0)},<br/>
{"yellow green", QColor(154,205,50)},<br/>
{"dark olive green", QColor(85,107,47)},<br/>
{"olive drab", QColor(107,142,35)},<br/>
{"lawn green", QColor(124,252,0)},<br/>
{"chart reuse", QColor(127,255,0)},<br/>
{"green yellow", QColor(173,255,47)},<br/>
{"dark green", QColor(0,100,0)},<br/>
{"green", QColor(0,128,0)},<br/>
{"forest green", QColor(34,139,34)},<br/>
{"lime", QColor(0,255,0)},<br/>
{"lime green", QColor(50,205,50)},<br/>
{"light green", QColor(144,238,144)},<br/>
{"pale green", QColor(152,251,152)},<br/>
{"dark sea green", QColor(143,188,143)},<br/>
{"medium spring green", QColor(0,250,154)},<br/>
{"spring green", QColor(0,255,127)},<br/>
{"sea green", QColor(46,139,87)},<br/>
{"medium aqua marine", QColor(102,205,170)},<br/>
{"medium sea green", QColor(60,179,113)},<br/>
{"light sea green", QColor(32,178,170)},<br/>
{"dark slate gray", QColor(47,79,79)},<br/>
{"teal", QColor(0,128,128)},<br/>
{"dark cyan", QColor(0,139,139)},<br/>
{"aqua", QColor(0,255,255)},<br/>
{"cyan", QColor(0,255,255)},<br/>
{"light cyan", QColor(224,255,255)},<br/>
{"dark turquoise", QColor(0,206,209)},<br/>
{"turquoise", QColor(64,224,208)},<br/>
{"medium turquoise", QColor(72,209,204)},<br/>
{"pale turquoise", QColor(175,238,238)},<br/>
{"aqua marine", QColor(127,255,212)},<br/>
{"powder blue", QColor(176,224,230)},<br/>
{"cadet blue", QColor(95,158,160)},<br/>
{"steel blue", QColor(70,130,180)},<br/>
{"corn flower blue", QColor(100,149,237)},<br/>
{"deep sky blue", QColor(0,191,255)},<br/>
{"dodger blue", QColor(30,144,255)},<br/>
{"light blue", QColor(173,216,230)},<br/>
{"sky blue", QColor(135,206,235)},<br/>
{"light sky blue", QColor(135,206,250)},<br/>
{"midnight blue", QColor(25,25,112)},<br/>
{"navy", QColor(0,0,128)},<br/>
{"dark blue", QColor(0,0,139)},<br/>
{"medium blue", QColor(0,0,205)},<br/>
{"blue", QColor(0,0,255)},<br/>
{"royal blue", QColor(65,105,225)},<br/>
{"blue violet", QColor(138,43,226)},<br/>
{"indigo", QColor(75,0,130)},<br/>
{"dark slate blue", QColor(72,61,139)},<br/>
{"slate blue", QColor(106,90,205)},<br/>
{"medium slate blue", QColor(123,104,238)},<br/>
{"medium purple", QColor(147,112,219)},<br/>
{"dark magenta", QColor(139,0,139)},<br/>
{"dark violet", QColor(148,0,211)},<br/>
{"dark orchid", QColor(153,50,204)},<br/>
{"medium orchid", QColor(186,85,211)},<br/>
{"purple", QColor(128,0,128)},<br/>
{"thistle", QColor(216,191,216)},<br/>
{"plum", QColor(221,160,221)},<br/>
{"violet", QColor(238,130,238)},<br/>
{"magenta", QColor(255,0,255)},<br/>
{"orchid", QColor(218,112,214)},<br/>
{"medium violet red", QColor(199,21,133)},<br/>
{"pale violet red", QColor(219,112,147)},<br/>
{"deep pink", QColor(255,20,147)},<br/>
{"hot pink", QColor(255,105,180)},<br/>
{"light pink", QColor(255,182,193)},<br/>
{"pink", QColor(255,192,203)},<br/>
{"antique white", QColor(250,235,215)},<br/>
{"beige", QColor(245,245,220)},<br/>
{"bisque", QColor(255,228,196)},<br/>
{"blanched almond", QColor(255,235,205)},<br/>
{"wheat", QColor(245,222,179)},<br/>
{"corn silk", QColor(255,248,220)},<br/>
{"lemon chiffon", QColor(255,250,205)},<br/>
{"light golden rod yellow", QColor(250,250,210)},<br/>
{"light yellow", QColor(255,255,224)},<br/>
{"saddle brown", QColor(139,69,19)},<br/>
{"sienna", QColor(160,82,45)},<br/>
{"chocolate", QColor(210,105,30)},<br/>
{"peru", QColor(205,133,63)},<br/>
{"sandy brown", QColor(244,164,96)},<br/>
{"burly wood", QColor(222,184,135)},<br/>
{"tan", QColor(210,180,140)},<br/>
{"rosy brown", QColor(188,143,143)},<br/>
{"moccasin", QColor(255,228,181)},<br/>
{"navajo white", QColor(255,222,173)},<br/>
{"peach puff", QColor(255,218,185)},<br/>
{"misty rose", QColor(255,228,225)},<br/>
{"lavender blush", QColor(255,240,245)},<br/>
{"linen", QColor(250,240,230)},<br/>
{"old lace", QColor(253,245,230)},<br/>
{"papaya whip", QColor(255,239,213)},<br/>
{"sea shell", QColor(255,245,238)},<br/>
{"mint cream", QColor(245,255,250)},<br/>
{"slate gray", QColor(112,128,144)},<br/>
{"light slate gray", QColor(119,136,153)},<br/>
{"light steel blue", QColor(176,196,222)},<br/>
{"lavender", QColor(230,230,250)},<br/>
{"floral white", QColor(255,250,240)},<br/>
{"alice blue", QColor(240,248,255)},<br/>
{"ghost white", QColor(248,248,255)},<br/>
{"honeydew", QColor(240,255,240)},<br/>
{"ivory", QColor(255,255,240)},<br/>
{"azure", QColor(240,255,255)},<br/>
{"snow", QColor(255,250,250)},<br/>
{"black", QColor(0,0,0)},<br/>
{"dim gray", QColor(105,105,105)},<br/>
{"gray", QColor(128,128,128)},<br/>
{"dark gray", QColor(169,169,169)},<br/>
{"silver", QColor(192,192,192)},<br/>
{"ight gray", QColor(211,211,211)},<br/>
{"gainsboro", QColor(220,220,220)},<br/>
{"white smoke", QColor(245,245,245)},<br/>
{"white", QColor(255,255,255)}};

[**Go to the previous page**](../../README.md)
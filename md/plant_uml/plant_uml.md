[**Go to the previous page**](../../README.md)

----

# Integration with PlantUML

DLT-Message-Analyzer's implementation is integrated with the PlantUML tool ( https://plantuml.com/ )
PlantUML is used to generate different types of UML diagrams out of their textual descriptions.
This plugin is using the PlantUML syntax in order to generate sequence diagrams out of the logs.

----

## Motivation

As a SW developer or SW architect, I want to be able to automatically build sequence diagrams out of the output of the different applications within the system. 
I want to be able to normalize data from different applications and put them all into a single unified sequence diagram.

Why is that important?
- Instant synchronization between logs and sequence diagrams is required in order to achieve productive discussion with different stakeholders, which often do not have idea regarding format of logs of different applications.
- Often the task is to represent, wchih PART OF THE SEQUENCE is wrong and should be changed. Building "correct" sequence based on the currently existing one is much faster, than building new sequence from scratch.
- Support of project's documentation becomes much easier when you can get sequences out of the latest logs. 

----

To turn on the PlantUML geature of the plugin, enable it in the general context-menu:

![Screenshot of PlantUML context menu item](./plant_uml_turn_on_feature.png)

----
> **Note!**
>
> PlantUML feature is turned on by default
----

The idea of this feature is that user can specify to the plugin, which parts means what in terms of a sequence diagram.
Predefined set of the regex group names is used to identify different common parts of the sequence diagram.
UML regex group identifiers, which are supported by the plugin:

| Group name | Meaning | Type |
| ------------- | ------------- | ------------- |
| USID | Sequence id of the communication | optional |
| UCL | Name of the client | mandatory |
| URT | Request identifier | request_type - at least one of the request types should be filled in |
| URS | Response identifier | request_type - at least one of the request types should be filled in |
| UEV | Event identifier | request_type - at least one of the request types should be filled in |
| US | Service name | mandatory |
| UM | Method name | mandatory |
| UA | Call arguments | optional |

----

----
> **Note!**
>
> You can get the same information if you will input the "uml-id" command into the debug console.
>
> ![Screenshot of the result of uml-id debug consolce's command execution](./plant_uml_uml_id_help.png)
----

Result formatting within the generated plantuml file would look like this:
<pre>&lt;UCL&gt; &lt;URT|URS|UE&gt; &lt;US&gt; : [timestamp] &lt;USID&gt;&lt;UM&gt;(&lt;UA&gt;)</pre>

In case if provided regex will contain ALL mandatory fields, plugin will consider them, and will check each found message regarding whether it can be used for creation of the sequence diagram.

Let's provide an example.
Imagine that we have the following 3 messages, which represent the communication between components "Alice" and "Bob":

![Screenshot of messages between Alice and Bob components](./plant_uml_alice_bob_messages.png)

Above way of tracing is unified, and contains all required parts to create a sequence out of it. 
We can use the following regex expression for sequence creation:
<pre>^(SYS).*?: Informational: (?&lt;UCL&gt;[\w]+):.*?((?&lt;URT&gt;RQ)|(?&lt;URS&gt;RP)|(?&lt;UEV&gt;EV)).*?(to|from) (?&lt;US&gt;[\w]+) - (?&lt;UM&gt;.*?)\((?&lt;UA&gt;.*?)\)</pre>

Result of search would look like:

![Screenshot of the result, which contains UML items](./plant_uml_search_result.png)

As you can see above, all found messages have active checkbox items in the "UML" column.
It means, that they can be used for diagram creation.
Let's select those rows:

![Screenshot of the result with selected UML items](./plant_uml_search_result_selected_UML_items.png)

----
> **Note!**
>
> As you can see, all selected rows will be highlighted with the gray color. That is done to increase readability during work with UML capabilities.
> 
> It is possible to select multiple rows to be used for UML diagram creation via "Selecting multiple rows by mouse, and then pressing Space"
> 
> It is possible to select all rows to be used for UML diagram creation via "Input Ctrl+A, then pressing Space"
----

Now, let's switch to the "UML view" and trigger diagram creation:

![Screenshot of triggering creation of UML sequence diagram](./plant_uml_trigger_creation_of_diagram.png)

The resulting diagram looks like this:

![Screenshot of the created UML sequences diagram](./plant_uml_alice_bob_diagram.png)

The resulting diagram can be saved as PNG, SVG or PUML file.

Use the following context menu item to trigger save operation:

![Screenshot of "Save as ..." context menu item](./plant_uml_save_diagram.png)

----
> **Note:**
>
> Default maximum number of rows within the rendered diagram - 1000 rows. All rows which are above that number will be ignored. Anyway, this limit can be adjusted from within UI, if needed.
> 
> ![Screenshot of "Max rows number ..." context menu item](./plant_uml_max_row_number.png)
----

----
> **Note:**
>
> By default the call arguments are shown. Row description is wrapped every 200 symbols.
> These parameters can be overwritten via context menu:
> 
> ![Screenshot of other UML parameters](./plant_uml_other_settings.png)
----

----

[**Go to the previous page**](../../README.md)
# Hi, guest. You are on the page of the DLTMessageAnalyzer plugin.

----
## What is the DLTMessageAnalyzer?

The DLTMessageAnalyzer is a plugin for dlt-viewer SW. It works together with the **[following source code](https://github.com/GENIVI/dlt-viewer)**

It is developed in order to increase the analytical capabilities of the dlt-viewer SW.

Feature-set:

- Search speed ten times faster, than in the original dlt-viewer ( in case of enabled "in-RAM" cache )
- Possibility to save regex patterns with the human-readable aliases
- Possibility to use combinations of the saved regex patterns, which allows to form complex requests without the need to remember the regex expressions.
This increadibly increases the speed of the trace analysis!
- This plugin uses the instances of all other available and turned-on decoding plugins, so it cache and analyze the decoded messages
- [Grouped view](./md/grouped_view/grouped_view.md). It allows you to easily form groups of repetitive messages and check request-response pairs, trace-spam cases, etc.
- "Search view". Analogue of the dlt-viewer's search with the extended capabilities
- "Files view". Extension, which shows names of the opened files
- Advanced highlighting of regex groups ( check [regex name scripting](./md/regex_name_scripting/regex_name_scripting.md) section )
- Continuous search. The analysis continues to happen while new messages being received from HU 
- Check of entered regular expressions with providing human-readable errors in case of wrong input
- In-RAM cache. Makes search dramatically faster ( check [in-RAM cache](./md/in_ram_cache/in_ram_cache.md) section )
- Possibility to work with multiple regex configuration files and switch between them. Allows to use other domains knowledge in your analysis
- "Filters view". Allows you to quickly get access to the defined "key varialbe regex groups" and change their content ( check [regex name scripting](./md/regex_name_scripting/regex_name_scripting.md) section )
- Lock of the search between 2 message id-s
- Debug "console view"<br/>
and many other features

**=> The main goal of this plugin is to fully replace the existing search functionality of the dlt-viewer, making it faster, more readable, intuitive, and efficient for any user.**

----

## Screenshots:

![Screenshot of DLTMessageAnalyzer plugin - Search view](./md/DLTMessageAnalyzer_screenshot_SearchView.png)
----
![Screenshot of DLTMessageAnalyzer plugin - Grouped view](./md/DLTMessageAnalyzer_screenshot_GroupedView.png)
----
![Screenshot of DLTMessageAnalyzer plugin - Filters view](./md/DLTMessageAnalyzer_screenshot_FilterView.png)
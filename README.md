# Hi, guest. You are on the page of the DLTMessageAnalyzer plugin.

----
## What is the DLTMessageAnalyzer?

The DLTMessageAnalyzer is a plugin for dlt-viewer SW. It works together with the **[following source code](https://github.com/GENIVI/dlt-viewer)**

It is developed in order to increase the analytical capabilities of the dlt-viewer SW.

## Feature-set:

### [In-RAM cache](./md/in_ram_cache/in_ram_cache.md)

- Makes search dramatically faster

### [Grouped view](./md/grouped_view/grouped_view.md)

- Allows you to easily form groups of repetitive messages and check request-response pairs, trace-spam cases, etc.

### [Regex name scripting](./md/regex_name_scripting/regex_name_scripting.md)

- Advanced highlighting of regex groups
- "Filters view". Allows you to quickly get access to the defined "key varialbe regex groups" and change their content
- Possibility to copy-paste highlighted content into your emails and bug-tracking systems

### [Patterns view](./md/patterns_view/patterns_view.md)

- Possibility to save regex patterns with the human-readable aliases
- Possibility to use combinations of the saved regex patterns, which allows to form complex requests without the need to remember the regex expressions.
This increadibly increases the speed of the trace analysis!
- Possibility to work with multiple regex configuration files and switch between them. Allows to use other domains knowledge in your analysis

### [Search view](./md/search_view/search_view.md)

- Analogue of the dlt-viewer's search with the extended capabilities
- Lock of the search between 2 message id-s

### [Files view](./md/files_view/files_view.md)

- Extension, which shows names of the all currently opened DLT files

### [Search](./md/search/search.md)

- Check of entered regular expressions with providing human-readable errors in case of wrong input
- Continuous search. The analysis continues to happen while new messages being received from HU 

### [Debug console](./md/debug_console/debug_console.md)

- Debug "console view"

and many other features

----

## Screenshots:

![Screenshot of DLTMessageAnalyzer plugin - Search view](./md/DLTMessageAnalyzer_screenshot_SearchView.png)
----
![Screenshot of DLTMessageAnalyzer plugin - Grouped view](./md/DLTMessageAnalyzer_screenshot_GroupedView.png)
----
![Screenshot of DLTMessageAnalyzer plugin - Filters view](./md/DLTMessageAnalyzer_screenshot_FilterView.png)
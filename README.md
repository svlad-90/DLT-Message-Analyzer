![DLT Message Analyzer logo](./md/DLTMessageAnalyzer_logo.png)

![Build linux](https://github.com/svlad-90/DLT-Message-Analyzer/workflows/Build%20linux/badge.svg)
![Build windows](https://github.com/svlad-90/DLT-Message-Analyzer/workflows/Build%20windows/badge.svg)
![Build clang-tidy](https://github.com/svlad-90/DLT-Message-Analyzer/workflows/Build%20clang-tidy/badge.svg)

----

## What is the DLT Message Analyzer?

The DLT Message Analyzer is a plugin for dlt-viewer SW. It works in combination with the **[following source code](https://github.com/GENIVI/dlt-viewer)**.

It is developed in order to increase the analytical capabilities of the dlt-viewer.

----

## Installation guide

### [Here](./md/installation_guide/installation_guide.md)

----

## Feature-set:

### [In-RAM cache](./md/in_ram_cache/in_ram_cache.md)

- Makes search dramatically faster

### [Grouped view](./md/grouped_view/grouped_view.md)

- Allows you to easily form groups of the repetitive messages and check trace-spam cases, request-response pairs, number of message occurrences, etc.

### [Regex name scripting](./md/regex_name_scripting/regex_name_scripting.md)

- Advanced highlighting of regex groups
- "Filters view". Allows you to quickly get access to the defined "key regex groups" and change their content
- Possibility to copy-paste highlighted content into your emails in order to increase its readability

### [Patterns view](./md/patterns_view/patterns_view.md)

- Possibility to save regex patterns with the human-readable aliases
- Possibility to use combinations of the saved regex patterns, which allows to instantly form the complex requests. This incredibly increases the speed and quality of the trace analysis!
- Possibility to work with multiple regex configuration files and switch between them. This allows you to use other's domains knowledge in your own analysis
- Possibility to search patterns by their aliases

### [Search view](./md/search_view/search_view.md)

- Analog of the dlt-viewer's search with the extended capabilities
- Possibility to lock the search between the 2 message id-s

### [Files view](./md/files_view/files_view.md)

- Extension, which lists the paths & names of all currently opened DLT files

### [Search](./md/search/search.md)

- Validation of the entered regular expressions with providing human-readable error in case of the wrong input
- Continuous search. The analysis ongoing, while new messages are being received from the target device 

### [Debug console](./md/debug_console/debug_console.md)

- Debug "console view", which prints different kinds of useful information.

and many other features

----

## Screenshots:

![Screenshot of DLT Message Analyzer plugin - Search view](./md/DLTMessageAnalyzer_screenshot_SearchView.png)
----
![Screenshot of DLT Message Analyzer plugin - Grouped view](./md/DLTMessageAnalyzer_screenshot_GroupedView.png)
----
![Screenshot of DLT Message Analyzer plugin - Filters view](./md/DLTMessageAnalyzer_screenshot_FilterView.png)

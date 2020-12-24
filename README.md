----

## Please, support!

<span style="color:red">If you are an active user of the DLT-Message-Analyzer plugin - please, spend a few moments to star this GitHub repository. Our team will very much appreciate it!</span>

----

![DLT Message Analyzer logo](./md/DLTMessageAnalyzer_logo.png)

![Build linux](https://github.com/svlad-90/DLT-Message-Analyzer/workflows/Build%20linux/badge.svg)
![Build windows](https://github.com/svlad-90/DLT-Message-Analyzer/workflows/Build%20windows/badge.svg)
![Build clang-tidy](https://github.com/svlad-90/DLT-Message-Analyzer/workflows/Build%20clang-tidy/badge.svg)

----

## What is DLT Message Analyzer?

DLT Message Analyzer is a plugin for dlt-viewer SW. It works in combination with the **[following source code](https://github.com/GENIVI/dlt-viewer)**.

It is developed to increase the analytical capabilities of the dlt-viewer.

----

## Installation guide

### [Read it here](./md/installation_guide/installation_guide.md)

----

## Feature-set:

### [Search](./md/search/search.md)

- Validation of the entered regular expressions with providing human-readable error in case of the wrong input
- Continuous search. The analysis ongoing, while new messages are being received from the target device 

### [In-RAM cache](./md/in_ram_cache/in_ram_cache.md)

- Makes search dramatically fast

### [Grouped view](./md/grouped_view/grouped_view.md)

- Allows you to easily form groups of the repetitive messages and check trace-spam cases, request-response pairs, number of message occurrences, etc.

### [Filters view](./md/filters_view/filters_view.md)

- Allows you to quickly get access to the defined "key regex groups" and change their content
- Provides auto-completion functionality

### [Advanced highlighting](./md/advanced_highlighting/advanced_highlighting.md)

- Advanced highlighting of regex groups
- Possibility to copy-paste highlighted content into your emails to increase their readability

### [Patterns view](./md/patterns_view/patterns_view.md)

- Possibility to save regex patterns with the human-readable aliases
- Possibility to use combinations of the saved regex patterns, which allows you to instantly form the complex requests. This incredibly increases the speed and quality of the trace analysis!
- Possibility to work with multiple regex configuration files and switch between them. This allows you to use other's domains knowledge in your analysis
- Possibility to search patterns by their aliases

### [Search view](./md/search_view/search_view.md)

- Analog of the dlt-viewer's search with the extended capabilities
- Possibility to lock the search between the 2 message id-s

### [Integration with PlantUML](./md/plant_uml/plant_uml.md)

- Plugin supports integration with the PlantUML tool, which allows you to create sequence diagrams out of the logs

### [Files view](./md/files_view/files_view.md)

- Extension, which lists the paths & names of all currently opened DLT files

### [Debug console](./md/debug_console/debug_console.md)

- Debug "console view", which shows debug messages of the plugin and allows you to perform a set of predefined scenarios.

and many other features

----

## Troubleshooting

### [Read it here](./md/troubleshooting/troubleshooting.md)

----

## Thirdparty dependencies: 

### [Read it here](./md/thirdparty_deps/thirdparty_deps.md)

----

## Development documentation

### [Read it here](./md/dev_docs/dev_docs.md)

----

## Repo size: 

### [Read it here](./md/repo_size/repo_size.md)

----

## Screenshots:

![Screenshot of DLT Message Analyzer plugin - Filters view](./md/DLTMessageAnalyzer_screenshot_FilterView.png)
----
![Screenshot of DLT Message Analyzer plugin - Grouped view](./md/DLTMessageAnalyzer_screenshot_GroupedView.png)
----
![Screenshot of DLT Message Analyzer plugin - UML view](./md/DLTMessageAnalyzer_screenshot_UMLView.png)

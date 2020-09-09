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
- Possibility to copy-paste highlighted content into your emails in order to increase their readability

### [Patterns view](./md/patterns_view/patterns_view.md)

- Possibility to save regex patterns with the human-readable aliases
- Possibility to use combinations of the saved regex patterns, which allows to instantly form the complex requests. This incredibly increases the speed and quality of the trace analysis!
- Possibility to work with multiple regex configuration files and switch between them. This allows you to use other's domains knowledge in your own analysis
- Possibility to search patterns by their aliases

### [Search view](./md/search_view/search_view.md)

- Analog of the dlt-viewer's search with the extended capabilities
- Possibility to lock the search between the 2 message id-s

### [Integration with PlantUML](./md/plant_uml/plant_uml.md)

- Plugin supports integration with the PlantUML tool, which allows you to create sequence diagrams out of the logs

### [Files view](./md/files_view/files_view.md)

- Extension, which lists the paths & names of all currently opened DLT files

### [Debug console](./md/debug_console/debug_console.md)

- Debug "console view", which prints different kinds of useful information.

and many other features

----

## Development documentation ( under construction )

### [Read it here](./md/dev_docs/dev_docs.md)

----

## Troubleshooting

### [Read it here](./md/troubleshooting/troubleshooting.md)

----

## Repo size:

Baseline: https://github.com/svlad-90/DLT-Message-Analyzer/commit/7c7319e444771a8ff7444085ec3dc59c17c98a54

| Language     | files | blank | comment | code |
| ---- | ---- | ---- | ---- | ---- |
| C++          | 38 | 3394 | 584 | 16703 |
| C/C++ Header   | 43 | 1321 | 1189 | 4598 |
| Markdown       | 26 | 677 | 0 | 1327 |
| Qt             | 1 | 8 | 0 | 1183 |
| CMake          | 15 | 76 | 71 | 404 |
| ANTLR Grammar  | 1 | 46 | 291 | 400 |
| YAML           | 4 | 46 | 33 | 206 |
| Java           | 3 | 46 | 9 | 125 |
| Maven          | 1 | 15 | 0 | 112 |
| HTML           | 1 | 0 | 0 | 1 |
| SUM:           | 133 | 5629 | 2177 | 25059 |

----

## Screenshots:

![Screenshot of DLT Message Analyzer plugin - Filters view](./md/DLTMessageAnalyzer_screenshot_FilterView.png)
----
![Screenshot of DLT Message Analyzer plugin - Grouped view](./md/DLTMessageAnalyzer_screenshot_GroupedView.png)
----
![Screenshot of DLT Message Analyzer plugin - UML view](./md/DLTMessageAnalyzer_screenshot_UMLView.png)
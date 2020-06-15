[**Go to the previous page**](../../README.md)

----

# Installation guide

- Clone the dlt-viewer project from the **[following location]( https://github.com/GENIVI/dlt-viewer )**.

----

> **Note!**
> 
> Currently plugin can be built against the v2.20.0 release. So, be sure, that you've checked out the correct base-line. 

----

- Follow instructions within the dlt-viewer's repo to have it successfully built.
- Clone DLT-Message-Analyzer's git repository ( the one which you are observing right now ) as a nested one inside the **"./dlt-viewer/plugin"** location.
Your target path to plugin should look like **"./dlt-viewer/plugin/DLT-Message-Analyzer"**:

![Screenshot of DLT Message Analyzer plugin location inside the dlt-viewer project](./installation_guide_plugin_location.png)

----

## "QT Creator"-specific part of guide.

- Modify the **"./dlt-viewer/plugin.pro"** in the following way:
<pre>SUBDIRS += DLT-Message-Analyzer ... all other plugin's names, which exist in delivery by default ...</pre>

![Screenshot of plugin.pro file's modification](./installation_guide_plugin_pro.png)

![Screenshot of plugin.pro cmakelists.txt modification](./installation_guide_cmakelists_modification.png)

- Run qmake:

![Screenshot of "Run qmake" QT's option](./installation_guide_run_qmake.png)

- Rebuild the dlt-viewer project: ![Screenshot of "Build project" QT's option](./installation_guide_build.png)

----

## CMake-specific part of guide

- In case if you build the project without the QT Creator, using only the CMakeLists.txt, modigy also the **"./dlt-viewer/plugin"**:

<pre>add_subdirectory(DLT-Message-Analyzer/dltmessageanalyzerplugin/src)</pre>

- Open console in the "./dlt-viewer" folder:

![Screenshot of console](./installation_guide_console.png)

- Run the following set of commands in it:

<pre>mkdir build
cd ./build
rm -r *
cmake ../
make
</pre>

----

- Proceed to the build artifact's folder and run the dlt-viewer, including dynamic library of the DLT-Message-Analyzer plugin
- Enable and show the DLT-Message-Analyzer plugin:

![Screenshot of enabling the plugin](./installation_guide_enable_plugin.png)

----

[**Go to the previous page**](../../README.md)
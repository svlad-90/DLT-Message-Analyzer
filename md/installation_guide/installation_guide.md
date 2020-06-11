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

- Modify the **"./dlt-viewer/plugin.pro"** in the following way:
<pre>SUBDIRS += DLT-Message-Analyzer ... all other plugin's names, which exist in delivery by default ...</pre>

![Screenshot of plugin.pro file's modification](./installation_guide_plugin_pro.png)

- Run qmake:

![Screenshot of "Run qmake" QT's option](./installation_guide_run_qmake.png)

- Rebuild the dlt-viewer project: ![Screenshot of "Build project" QT's option](./installation_guide_build.png)

- Run dlt-viewer, including the dynamic library of the DLT-Message-Analyzer plugin
- Enable the DLT-Message-Analyzer plugin

----

[**Go to the previous page**](../../README.md)
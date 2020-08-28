[**Go to the previous page**](../../README.md)

----

# Troubleshooting

## Instance of dlt-viewer has an old-fashion styling

Majority of the users are using dlt-viewer with the default styling, which is "windowsvista".

If you build dlt-viewer with Qt version which is equal or greater than 5.10, you should be aware that "windowsvista" styling has become a separate dynamic library there.

In order to check whether it is still available or not you can switch to the "Console view" and input "styles".
That will show you used, and available style sets:

![Screenshot of the "styles" command output](./troubleshooting_styles_command.png)

You can select one of the available styles using the "-style" command line option, e.g.:

> ./dlt-viewer -style Fusion

In case if "windowsvista" style is missing in the above list, you can add it to the final deployment of the dlt-viewer in the following way:
> Take it from: "&lt;Qt_ROOT&gt;\\&lt;Qt_version&gt;\\&lt;Qt_toolchain&gt;\plugins\styles\qwindowsvistastyle.dll" // or *.so, depending on the used OS.
>
> Place it to: ".\dlt-viewer\styles\qwindowsvistastyle.dll" // or *.so, depending on the used OS.

Then reboot the dlt-viewer. Addition style should become available.

----

[**Go to the previous page**](../../README.md)
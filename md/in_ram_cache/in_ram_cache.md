[**Go to the previous page**](../../README.md)

----

# In-RAM cache

----

DLT Message Analyzer plugin supports the "in-RAM cache", which means, that the user can pre-cache the whole analyzed file or its part within the RAM, to achieve the faster search.

By default, DLT-viewer access the storage ( HDD, SSD ) to read out each message during each search operation.
For sure, that is quite slow if we will compare it against the "search within the RAM".
Observed boost of the DLT Message Analyzer's search speed against the dlt-viewer's one is around "5 times faster" on the SSD and "10 times faster" on the HDD. 

That matters, when we are talking about analysis of the 500 Mb file, where each search operation might take 5.5 seconds or 55 seconds.

Cache has the lazy initialization strategy, so it will be filled in during indexing of the DLT file, or during the first search procedure.
In case if the file size is bigger than the specified cache limit, only a part of the file, which fits the specified limit will be stored into the cache. Other parts will be accessed from the drive.

----

GUI part which describes the status of the cache looks like this:

![Screenshot of in-RAM cache status](./cache_status.png)

It shows:
- Whether cache is enabled?
- What is the cache size?
- What is the used cache in percent?
- What is the used cache part in megabytes?

----

The "in-RAM cache" related settings are located within the following context menu:

![Screenshot of in-RAM cache context menu](./context_menu.png)

It allows you to:
- Disable or enable cache
- Specify the cache size
- Reset the cache

----

> **Note!**
>
> Be aware, that the actual RAM consumption of the cache is ~X*2.5 from what the status is showing.
> The thing is that the cache size in the status message is measured as the sum of the sizes of all fetched payloads and headers, which are stored as blobs. 
> But the cache itself is stored in a slightly different, non-blob data-structure, which causes additional RAM consumption.

----

> **Useful hint!**
>
> In case if you are analyzing the huge DLT file, e.g. 10 Gb, it is quite useful to:
> - turn off the cache
> - use search without the cache at least once to find the range of time in which you are interested
> - lock the search range to the target area
> - turn on the cache
>
> In such a way you won't get the "lack of the RAM" use-case while using the cache, as it will pre-cache only a range, which you are interested in.

----

> **Note!**
>
> Plugin checks the provided cache limit against the actual amount of RAM on the client's machine.
> Still cache does not monitor the currently available amount of RAM.
> Thus, if you will specify e.g. 32Gb as cache size having 32Gb on your machine, it might happen that you will face a lack of RAM use-case and dlt-viewer will crash.
> Be aware of that.

----

Extensive custom highlighting & grouping might slow down the search a little bit.
That impact is observed due to run-time calculations and sorting.
Still, even in worst cases, processing is done much faster than in the usual dlt-viewer's search functionality. 

[**Go to the previous page**](../../README.md)
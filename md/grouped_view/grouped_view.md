[**Go to the previous page**](../../README.md)

----

# Grouped view

The intention of the "grouped view" is to allow to split messages into the groups, based on the "regex groups" syntax. 
Those groups are then represented in a form of the tree view.

The view provides the following information regarding each tree level:
- number of messages, entries
- number of messages, percents
- number of messages, average per second
- payload size, bytes
- payload size, percents
- payload size, average per second

# How does it look like?

![Screenshot of the "grouped view"](./grouped_view_appearence.png)

# How does it work?

The above screenshot contains an example of "grouped view" for a system journal's messages.

### Used regex is:

<pre>^(SYS) ([A-Z0-9]{1,4}) [0-9]+/[0-9]+/[0-9]+ [0-9]+:[0-9]+:[0-9]+\.[0-9]+ (?&lt;VAR_SYS_MES_APP&gt;[\w_-]+).*: (?&lt;VAR_SYS_MES_TYPE&gt;[A-Za-z_-]+): \[[\d]+\.[\d]+\](.*)</pre>

### The grouped view will form the following set of the tree-view groups:

- (SYS) - group for the specific dlt app-id SYS
- ([A-Z0-9]{1,4}) - sub-group for the ANY dlt context
- (?&lt;VAR_SYS_MES_APP&gt;[\w_-]+) - sub-group for ANY service name
- (?&lt;VAR_SYS_MES_TYPE&gt;[A-Za-z_-]+) - sub-group for ANY message type
- (.*) - sub-group for ANY message

### The result will be something like this:

<pre>Root | Msg-s : 110 | Msg-s, % : 100.000 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 100.000 | Payload, b/sec, av. : 135
|-SYS | Msg-s : 110 | Msg-s, % : 100.000 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 100.000 | Payload, b/sec, av. : 135
  |-JOUR | Msg-s : 110 | Msg-s, % : 100.000 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 100.000 | Payload, b/sec, av. : 135
    |-dlt-daemon | Msg-s : 110 | Msg-s, % : 100.000 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 100.000 | Payload, b/sec, av. : 135
      |-Informational | Msg-s : 110 | Msg-s, % : 100.000 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 100.000 | Payload, b/sec, av. : 135
        |-~DLT~61956~NOTICE   ~Send log-level to context: SYS:JOUR [-1 -> 4] [-1 -> 0] | Msg-s : 19 | Msg-s, % : 17.273 | Msg-s/sec, av. : 0 | Payload : 4921 | Payload, % : 17.836 | Payload, b/sec, av. : 24
        |-~DLT~61956~NOTICE   ~Send log-level to context: SYS:MGR [-1 -> 4] [-1 -> 0] | Msg-s : 19 | Msg-s, % : 17.273 | Msg-s/sec, av. : 0 | Payload : 4902 | Payload, % : 17.767 | Payload, b/sec, av. : 24
        |-~DLT~61956~NOTICE   ~Send log-level to context: SYS:SYSL [-1 -> 4] [-1 -> 0] | Msg-s : 19 | Msg-s, % : 17.273 | Msg-s/sec, av. : 0 | Payload : 4921 | Payload, % : 17.836 | Payload, b/sec, av. : 24
        |-~DLT~61956~INFO     ~Activate connection type: 2 | Msg-s : 10 | Msg-s, % : 9.091 | Msg-s/sec, av. : 0 | Payload : 2310 | Payload, % : 8.373 | Payload, b/sec, av. : 11
        |-~DLT~61956~INFO     ~Deactivate connection type: 2 | Msg-s : 10 | Msg-s, % : 9.091 | Msg-s/sec, av. : 0 | Payload : 2330 | Payload, % : 8.445 | Payload, b/sec, av. : 11
        |-~DLT~61956~INFO     ~Switched to buffer state for socket connections. | Msg-s : 10 | Msg-s, % : 9.091 | Msg-s/sec, av. : 0 | Payload : 2520 | Payload, % : 9.134 | Payload, b/sec, av. : 12
        |-~DLT~61956~INFO     ~Switched to send buffer state for socket connections. | Msg-s : 10 | Msg-s, % : 9.091 | Msg-s/sec, av. : 0 | Payload : 2570 | Payload, % : 9.315 | Payload, b/sec, av. : 12
        |-~DLT~61956~INFO     ~Switched to send direct state. | Msg-s : 10 | Msg-s, % : 9.091 | Msg-s/sec, av. : 0 | Payload : 2340 | Payload, % : 8.481 | Payload, b/sec, av. : 11
        |-~DLT~61956~NOTICE   ~Send log-level to context: SYS:JOUR [-1 -> 4] [-1 -> 1] | Msg-s : 1 | Msg-s, % : 0.909 | Msg-s/sec, av. : 0 | Payload : 259 | Payload, % : 0.939 | Payload, b/sec, av. : 1
        |-~DLT~61956~NOTICE   ~Send log-level to context: SYS:MGR [-1 -> 4] [-1 -> 1] | Msg-s : 1 | Msg-s, % : 0.909 | Msg-s/sec, av. : 0 | Payload : 258 | Payload, % : 0.935 | Payload, b/sec, av. : 1
        |-~DLT~61956~NOTICE   ~Send log-level to context: SYS:SYSL [-1 -> 4] [-1 -> 1] | Msg-s : 1 | Msg-s, % : 0.909 | Msg-s/sec, av. : 0 | Payload : 259 | Payload, % : 0.939 | Payload, b/sec, av. : 1
</pre>
 
 As you can see, result formed tree levels for:
 - SYS app id
 - JOUR context
 - dlt-damon service
 - Information message type
 - Messages
 
The tested file was a quite small one, which contained system journal messages only from the dlt-daemon.
Still even in that data-set we can see, that there were 19 same messages from dlt-daemon regarding sending log-level contexts.
And 10 desctivation of connection type 2.

Together with that you can see which kind of message has taken which amount of payload.
That is quite important in case if you work within a system, which has a trace-spam cases, which should be addressed.

----

# Trace spam use-case

### How it looks like?

![Screenshot of the "trace-spam use-case"](./grouped_view_trace_spam.png)

### Used regex is:

<pre>^(?&lt;VAR_TRACE_SPAM_APP&gt;[A-Z0-9]{1,4}) (?&lt;VAR_TRACE_SPAM_CONTEXT&gt;[A-Z0-9]{1,4})(?&lt;VAR_TRACE_SPAM_50_CHARS&gt;.{0,50})(?&lt;VAR_TRACE_SPAM_REST_MSG&gt;.*)</pre>

### The grouped view will form the following set of the tree-view nodes:

- ^(?&lt;VAR_TRACE_SPAM_APP&gt;[A-Z0-9]{1,4}) - group for ANY dlt app-id
- (?&lt;VAR_TRACE_SPAM_CONTEXT&gt;[A-Z0-9]{1,4}) - sub-group for ANY dlt context
- (?&lt;VAR_TRACE_SPAM_50_CHARS&gt;.{0,50}) - sub-group for first 50 characters of ANY message
- (?&lt;VAR_TRACE_SPAM_REST_MSG&gt;.*) - sub-group for rest of the message

### The result will be something like this:

<pre>Root | Msg-s : 258 | Msg-s, % : 100.000 | Msg-s/sec, av. : 0 | Payload : 49917 | Payload, % : 100.000 | Payload, b/sec, av. : 0
|-SYS | Msg-s : 110 | Msg-s, % : 42.636 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 55.272 | Payload, b/sec, av. : 0
  |-JOUR | Msg-s : 110 | Msg-s, % : 42.636 | Msg-s/sec, av. : 0 | Payload : 27590 | Payload, % : 55.272 | Payload, b/sec, av. : 0
|-DA1 | Msg-s : 74 | Msg-s, % : 28.682 | Msg-s/sec, av. : 0 | Payload : 11390 | Payload, % : 22.818 | Payload, b/sec, av. : 0
  |-DC1 | Msg-s : 74 | Msg-s, % : 28.682 | Msg-s/sec, av. : 0 | Payload : 11390 | Payload, % : 22.818 | Payload, b/sec, av. : 0
    |- [connection_info ok] 01 00 00 00 00 | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 1410 | Payload, % : 2.825 | Payload, b/sec, av. : 0
    |- [connection_info ok] 02 00 00 00 00 | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 1410 | Payload, % : 2.825 | Payload, b/sec, av. : 0
    |- [get_software_version ok] DLT Package Version: 2. | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 2730 | Payload, % : 5.469 | Payload, b/sec, av. : 0
    |- [set_default_log_level ok]  | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 1330 | Payload, % : 2.664 | Payload, b/sec, av. : 0
    |- [set_default_trace_status ok]  | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 1360 | Payload, % : 2.725 | Payload, b/sec, av. : 0
    |- [set_timing_packets ok]  | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 1300 | Payload, % : 2.604 | Payload, b/sec, av. : 0
    |- [set_verbose_mode not_supported]  | Msg-s : 10 | Msg-s, % : 3.876 | Msg-s/sec, av. : 0 | Payload : 1390 | Payload, % : 2.785 | Payload, b/sec, av. : 0
    |- [ error]  | Msg-s : 4 | Msg-s, % : 1.550 | Msg-s/sec, av. : 0 | Payload : 460 | Payload, % : 0.922 | Payload, b/sec, av. : 0
|-APP | Msg-s : 50 | Msg-s, % : 19.380 | Msg-s/sec, av. : 0 | Payload : 7115 | Payload, % : 14.254 | Payload, b/sec, av. : 0
  |-CON | Msg-s : 50 | Msg-s, % : 19.380 | Msg-s/sec, av. : 0 | Payload : 7115 | Payload, % : 14.254 | Payload, b/sec, av. : 0
|-DLTD | Msg-s : 20 | Msg-s, % : 7.752 | Msg-s/sec, av. : 0 | Payload : 3170 | Payload, % : 6.351 | Payload, b/sec, av. : 0
|-HATS | Msg-s : 4 | Msg-s, % : 1.550 | Msg-s/sec, av. : 0 | Payload : 652 | Payload, % : 1.306 | Payload, b/sec, av. : 0
  |-HADT | Msg-s : 4 | Msg-s, % : 1.550 | Msg-s/sec, av. : 0 | Payload : 652 | Payload, % : 1.306 | Payload, b/sec, av. : 0</pre>
  
[**Go to the previous page**](../../README.md)
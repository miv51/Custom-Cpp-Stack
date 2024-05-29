# Implementation Details
A low-level explaination of each module.

## Table of Contents
#### 1) Explaination of source code

<br/></br>

## Part 1 : Explaination of source code
Explainations and demonstrations of the most significant lower-level modules. All files referenced are in the <code/>examples</code> folder.

#### Json Utilities
This module is used to parse json objects with known structure. It includes a function for parsing a single json object and a method for parsing json arrays separately. <code/>json_parse.cpp</code> and <code/>json_array_parse.cpp</code> contain an example of parsing a single json object and array respectively. The examples should yield the following outputs.

<br> The following output is for the single json object.

```bash
Original Json Object - {"key0":"value0", "key1":3.1415, "key2":null}

Parsed Json Object key : value pairs
key0 : value0
key1 : 3.1415
key2 : null

Original Json Object - {"jsonArray":[0, 1, 2, 4, -6], "nestedJsonObject":{"a":"Hello World!", "b":false}}

Parsed Json Object key : value pairs
jsonArray : 0, 1, 2, 4, -6
nestedJsonObject : "a":"Hello World!", "b":false
```

<br> The following output is for the array of json objects.

```bash

s : FAKE
t : 2001-05-11T:09:42:00Z
v : 10295
n : 205
c : 22.05
o : 21.77
h : 22.25
l : 21.6

s : BOGUS
t : 2001-05-11T:11:25:00Z
v : 328166
n : 622
c : 4
o : 3.5
h : 4.2
l : 3.48
```

#### Websocket Utilities
This module is used to listen for updates from data streams. It is a basic implementation of [The Websocket Protocol](https://datatracker.ietf.org/doc/html/rfc6455) that does not handle continuation frames - since there shouldn't be any for my current user cases. Non-blocking I/O is also supported; in that case, when <code/> websocket::recv </code> is called, it will return immediately if it receives no frame header and will read the full message before returning otherwise. If it is a blocking socket, it will return once it has recieved a message. <br>

<code/>single_ws_blocking.cpp</code> and <code/>multiple_ws_non_blocking.cpp</code> contain an example of printing messages from a single blocking websocket and multiple non-blocking websockets respectively. Both of these examples use the yahoo finance data stream which sends proto-buffered messages, and these messages are not readable in the form that they are sent so if you decide to test those two examples then expect that.

#### Http Utilities
This module is used to perform http get, patch, post, and delete requests. Like the websocket module, the http module also supports both blocking and non-blocking I/O. <code/>single_get_request.cpp</code> and <code/>multiple_get_requests.cpp</code> contain an example performing a single http get request and multiple asynchronous get requests. <br>

#### Socket Utilities
This module is used to manage SSL resources and wrap sockets and socket operations. The websocket and http modules heavily utilize this module. <br>

#### Input-Output (io) Utilities
This module is used to convert strings to their respective datatypes. The convert function replaces io functions from the standard library such as std::stod, std::stoi, and other similar functions and the convertUTC function is used to convert UTC timestamps to time-of-day in nanoseconds (not time since epoch). <br>

#### Static Unordered Map (sumap) Utilities
This module hosts an unordered map class that only uses stack memory. <br>
FOR MY TRADING BOT AND STOCK SCANNER: Since we can closely estimate the number of stocks that the bot/scanner will watch every day, we can use a container that only uses stack memory to reduce the transversal and retrieval time. Since the bot/scanner listens to trade and quote updates, the container that contains the data for individual stocks will be accessed extremely frequently. I made this container to replace the std::unordered_map for this specific purpose (I still use std::unordered_map for many other things in the bot/scanner). <br>

#### Segmented Queue (sq) Utilities
This module hosts a queue that creates, joins, and frees blocks of memory in order to expand and contract. It serves the same purpose as std::queue but can have significantly faster appending, removing, and transversal (including with iterators) times depending on the size of the memory stacks used to construct arrays. Using this structure over std::queue offers the greatest performance boost for the critical path in some of my applications. <br>

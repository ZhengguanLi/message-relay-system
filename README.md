# Messay Delay System

The `message delay system` implements a simple `message relay client-server` application using C language (Unix/Linux machine). The sender process accepts user-entered "messages" from the keyboard, and sends these messages to the receiver process via the message relay process. The receiver process displays the message and performs checkSum once the message has been received entirety.

## Usage

### Compile

- Sender
    ```
    gcc sender.c -o sender.o
    ```

- Server
    ```
    gcc server.c -o server.o
    ```

- Receiver
    ```
    gcc receiver.c -o receiver.o
    ```

### Run

- Server
    ```
    ./server.o [port number]
    ```

- Sender
    ```
    ./sender.o [IP address/localhost] [port number]
    ```


- Receiver
    ```
    ./receiver.o [port number]
    ```

## Running Test

- Server
    ```
    $ ./server.o 55397
    Socket created!
    Bind completed!
    --------------------------------------
    Socket created!
    Connect successful!
    ```

- Sender
    ```
    $ ./sender.o localhost 55397
    Socket created!
    Connect successful!
    --------------------------------------
    Please enter the username: apple
    Acknowledged!
    Please enter the password: as
    Verified!
    Please enter the receiver name: gpel8.cs.ou.edu
    Please enter the receiver port number: 55398
    Verified!
    Relay server connected to receiver!
    --------------------------------------
    Please enter your messsage: Hi
    Message Complete!
    --------------------------------------
    Please enter your messsage: Hello
    Message Complete!
    ```

- Receiver
    ```
    $ ./receiver.o 55398
    Socket created!
    Bind completed!
    --------------------------------------
    message: Hi
    Message complete!
    message: Hello
    Message complete!
    ```

## Authors

- Zhengguan Li
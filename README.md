# Home-Brew-Web-Server

Homework 5: A Home-Brew Multi-Process Web Server
Homework objectives:

Learn how to follow a network protocol (in this case, http)
Learn network/socket programming
Learn some simple multithreading
For this lab we’ll build a simple multi-threaded Web server. As usual, start by creating your repository by using this link: https://classroom.github.com/assignment-invitations/ec0ef59987f5df3c88b2900a25a49c46

The hw5 directory contains:

homework5.c: Skeleton code for the server side of a TCP application. This will be the primary file for this assignment.  All your code should go in this file.
WWW/: A directory containing example files for your Web server to distribute.
thread_example.c: Example code that illustrates a very simple threaded programming scenario. You are not required to use or make any changes to this file, but you should understand what it does.
A Makefile.
Your server program will receive two arguments: 1) the port number it should listen on for incoming connections, and 2) the directory out of which it will serve files (often called the document root in production Web servers). For example:

$ ./homework5 8080 WWW
This command will tell your Web server to listen for connections on port 8080 and serve files out of the WWW directory. That is, the WWW directory is considered ‘/’ when responding to requests. For example, if you’re asked for /index.html, you should respond with the file that resides in WWW/index.html. If you’re asked for /dir1/dir2/file.ext, you should respond with the file WWW/dir1/dir2/file.ext.

Requirements

In addition to serving requested files, your server should handle at least the following cases:

HTML, text, and image files should all display properly. You’ll need to return the proper HTTP Content-Type header in your response, based on the file ending. Also, other binary file types like PDFs should be handled correctly. You don’t need to handle everything on that list, but you should at least be able to handle files with html, txt, jpeg, gif, png, and pdf extensions.
If asked for a file that does not exist, you should respond with a 404 error code with a readable error page, just like a Web server would. It doesn’t need to be fancy, but it should contain some basic HTML so that the browser renders something and makes the error clear.
Some clients may be slow to complete a connection or send a request. Your server should be able to serve multiple clients concurrently, not just back-to-back. For this lab, use multithreading with pthreads to handle concurrent connections. 
If the path requested by the client is a directory, you should handle the request as if it was for the file “index.html” inside that directory. Hint: use the stat() system call to determine if a path is a directory or a file. The st_mode field in the stat struct has what you need.
The Web server should respond with a list of files when the user requests a directory that does not contain an index.html file. You can read the contents of a directory using the opendir() and readdir() calls. Together they behave like an iterator, that is, you can open a (DIR *) with opendir and then continue calling readdir(), which returns info for one file, on that (DIR *) until it returns NULL. Note that there should be no additional files created on the server’s disk to respond to the request. The response should mimic result of running python -m SimpleHTTPServer
When testing, you should be able to retrieve byte-for-byte copies of files from your server. Use wget or curl to fetch files and md5sum or diff to compare the fetched file with the original. I will do this when grading. For full credit, the files need to be exact replicas of the original.

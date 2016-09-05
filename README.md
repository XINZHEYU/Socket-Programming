# Socket-Programming

Description of my code and development environment

In my program, server can listen to connection request and receive message from each connected socket. When server receive connect request, it creates a new thread to handle the login authentication, and the main thread go on to listen to other connected sockets. When receive a new message from client, server judge the command and call specific function to execute that command.

Instructions on how to run my code

(a) format of send message: 
	broadcast message
	message user message
	broaduser<user><user><user>…<user>message
	wholast minute
(b) TIME_OUT’s unit is second	BLOCK_TIME’s unit is second

Sample commands to invoke my code

After enter “make”, there will be two executable file in the folder, one is server and the other is client. Open them(you can open multiple clients). Then you can invoke them. The format of invoking is:
	server: port               ex. 4119
	client: IP port            ex. 127.0.0.1 4119

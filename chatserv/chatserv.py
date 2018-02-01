"""
This is a very simple chat server.
"""

import socket
import threading
import datetime
import time
import random
from operator import attrgetter

SERVER_HOST = '0.0.0.0'
SERVER_PORT = 47629
SERVER_ADDR = (SERVER_HOST, SERVER_PORT)

LIMITS_LOGSIZE = 40
LIMITS_USERNAME_LENGTH = 10
LIMITS_MESSAGE_LENGTH = 256

class User:
    """User class"""
    def __init__(self, username, client, connectTime, address, channel):
        """User constructor"""
        self.name = username
        self.client = client
        self.connectTime = connectTime
        self.address = address[0]
        self.channel = channel
        self.chanJoinTime = None

    def change_username(self, newUser):
        """Changes the username, if new name not already in use."""
        # User name exists, don't do anything
        if (Server.find_user(newUser) != None):
            self.client.send(("Username " + newUser + " already exists.\n").encode())
        # User doesn't exist, switch it!
        else:
            oldUser = self.name
            self.name = newUser
            print('{}: {} changed username to {}.'.format(datetime.datetime.now().time(), oldUser, newUser))
            self.channel.broadcast((oldUser + " changed username to " + newUser))

    def who_command(self):
        """Handles our /who information for a single user"""
        whoString = ''
        # Display user information
        whoString += ("WHO information for:\t" + self.name + "\n")
        whoString += ("Client address:\t\t" + self.address + "\n")
        whoString += ("Current channel:\t" + self.channel.name + "\n")
        whoString += ("Online since:\t\t" + self.connectTime + "\n")
        whoString +=("-- END OF /WHO " + self.name + " --\n")
        return whoString

    def update_chan_join_time(self):
        self.chanJoinTime = int(time.time())


class Server:
    """Server class, not instantiated"""
    clientlist = []
    channellist = []
    name = "CHATSERV"
    default_channel = None

    def add_user(user):
        """Add user to user list and report to log"""
        Server.clientlist.append(user)
        print('{}: {} added to current user table.'.format(datetime.datetime.now().time(), user.name))

    def remove_user(user):
        """Remove user from user list and report to log"""
        Server.clientlist.remove(user)
        print('{}: {} removed from current user table.'.format(datetime.datetime.now().time(), user.name))

    def add_channel(channelName, channelTopic, owner):
        """Create new channel and add to channel list"""
        if (channelName[0] != "#"):
            channelName = "#" + channelName
        newChannel = Channel(channelName, channelTopic, owner)
        Server.channellist.append(newChannel)
        newChannel.chat(newChannel, ("TOPIC: " + channelTopic))
        print('{}: {} added to channel table.'.format(datetime.datetime.now().time(), channelName))

        if (Server.default_channel == None):
            Server.default_channel = newChannel
        return newChannel

    def remove_channel(channel):
        """Remove channel from channel list"""
        # Put all users in default channel and remove channel
        for user in channel.userlist:
            Server.default_channel.join_channel(user)
        Server.channellist.remove(channel)
        print('{}: {} removed from channel table.'.format(datetime.datetime.now().time(), channel.name))

    def get_client_count():
        """Retrieves number of clients connected to the server"""
        return (len(Server.clientlist))

    def time_string():
        """Time string for all the time stuff we print."""
        return("<" + datetime.datetime.now().strftime('%H:%M:%S') + "> ")
    
    def broadcast(broadcast, userlist):
        """Broadcasts messages to all members of the given userlist"""
        curTime = Server.time_string()
        for user in userlist:
            user.client.send((curTime + broadcast + "\n").encode())
    
    def find_user(userName):
        """Helper function for finding users."""
        user = [x for x in Server.clientlist if x.name == userName]
        if (len(user) > 0):
            return user[0]
        else:
            return None

    def find_channel(channelName):
        """Find if channel is in channel list"""
        if (channelName[0] != "#"):
            channelName = "#" + channelName
        channel = [x for x in Server.channellist if x.name == channelName]
        if (len(channel) > 0):
            return channel[0]
        else:
            return None
        
    def who_command():
        """Handles our /who information for server users"""
        whoString = ''
        # Display all users
        whoString += (str(Server.get_client_count()) + " users connected to the server.\n")
        whoString += ("User Name\t\tChannel\t\tOnline Since\n")
        for user in Server.clientlist:
            whoString += (user.name + "\t\t")
            whoString += (user.channel.name + "\t\t")
            whoString += (user.connectTime + "\n")
        whoString +=("-- END OF /WHO ALL --\n")
        return whoString

    def kill_client(user):
        """Destroys all trace of the client on exit. Bye Felicia."""
        # Remove the user from the user table and current channel
        try:
            Server.remove_user(user)
            user.channel.leave_channel(user)
        except:
            fail = True

        # Shutdown the socket
        try:
            user.client.shutdown(socket.SHUT_RDWR)
            user.client.close()
        except:
            print("They left wrong. What a jerk.")

    def message_user(sender, receiverName, message):
        """Send a private message to another client."""
        user = Server.find_user(receiverName)
        if (user != None):
            user.client.send((Server.time_string() + "[PRIVATE MESSAGE] " + sender.name + ": " + message + "\n").encode())
            sender.client.send(("[PRIVATE MESSAGE] to " + receiverName + " sent.\n").encode())
        else:
            sender.client.send((receiverName + " not found.\n").encode())

    def help():
        """Print the server help message."""
        helpString = ''
        helpString += ("General commands:\n")
        helpString += ("/exit\t\t\t- disconnects from the server.\n")
        helpString += ("/join #channel\t\t- switches to a different channel, creating one if it doesn't exist.\n")
        helpString += ("/kick user\t\t- kicks a user from your channel (owner only).\n")
        helpString += ("/msg user message\t- sends a private message to that user.\n")
        helpString += ("/topic\t\t\t- displays the current channel topic.\n")
        helpString += ("/topic newTopic\t\t- changes the topic of the channel (owner only).\n")
        helpString += ("/user newUser\t\t- changes username to newUser\n")
        helpString += ("/who\t\t\t- displays all users online.\n")
        helpString += ("/who user\t\t- gets info for a specified user.\n")
        helpString += ("/who #channel\t\t- gets info for a specified channel.\n")
        helpString += ("Messages may be up to " + str(LIMITS_MESSAGE_LENGTH) + " characters long, including commands entered.\n\n")
        return helpString


class Channel:
    """Our channel class for each channel"""

    def __init__(self, channelname, channeltopic, owner):
        """Channel constructor"""
        if (channelname[0] != "#"):
            channelname = "#" + channelName
        self.name = channelname
        self.topic = channeltopic
        self.userlist = []
        self.backlog = []
        self.owner = owner

    def get_user_count(self):
        """Return number of users in channel"""
        return (len(self.userlist))

    def connection_message(self):
        """First message when connecting to the channel"""
        connectionMsg = ''
        connectionMsg += ("Channel owner: " + self.owner.name + "\n")
        connectionMsg += ("Channel topic: " + self.topic + "\n")
        connectionMsg += ("Last " + str(LIMITS_LOGSIZE) + " lines in chat:\n")
        
        for msg in self.backlog:
            connectionMsg += msg
        return connectionMsg

    def broadcast(self, broadcast):
        """Broadcast to all users in the channel"""
        broadcast = ("[" + self.name + "] " + broadcast)
        Server.broadcast(broadcast, self.userlist)
        self.backlog.append((Server.time_string() + broadcast + "\n"))

        if (len(self.backlog) > LIMITS_LOGSIZE):
            self.backlog.pop(0)


    def change_topic(self, newTopic):
        """Change the channel topic and broadcast it"""
        self.topic = newTopic
        self.broadcast(("Channel topic: " + newTopic), self.userlist)

    def kick_user(self, kicker, username):
        """Kick a user from a channel and tell them who did it."""
        if (kicker != self.owner):
            return -1
        kickee = Server.find_user(username)
        if (kickee != None):
            kickee.channel = Server.default_channel
            self.userlist.remove(kickee)
            self.broadcast((username + " has been kicked from the channel by " + kicker.name + "."))
            return 0
        else:
            return None

    def who_command(self):
        """Handles our /who information for channel users"""
        whoString = ''
        # Display users in channel
        whoString += (str(self.get_user_count()) + " users in " + self.name + "\n")
        whoString += ("User Name\t\tOnline Since\n")
        for user in self.userlist:
            whoString += (user.name + "\t\t")
            whoString += (user.connectTime)
            if (user == self.owner):
                whoString += (" [OWNER]\n")
            else:
                whoString += ("\n")
        whoString +=("-- END OF /WHO " + self.name + " --\n")
        return whoString

    def chat(self, user, chatMsg):
        """Send chat to the channel and put it in the backlog"""
        Server.broadcast((user.name + ": " + chatMsg), self.userlist)
        self.backlog.append((Server.time_string() + user.name + ": " + chatMsg + "\n"))
        
        if (len(self.backlog) > LIMITS_LOGSIZE):
            self.backlog.pop(0)

    def leave_channel(self, user):
        """Remove a user from the channel list and tell everyone"""
        user.channel = None
        self.userlist.remove(user)
        
        if (len(self.userlist) > 0):
            self.broadcast((user.name + " has left the channel."))
        if ((self.owner == user) and (len(self.userlist) > 0)):
            newOwner = self.find_earliest_user()
            self.owner = newOwner
            self.broadcast((newOwner.name + " is now the owner of [" + self.name + "]."))
        elif ((len(self.userlist) == 0) and (Server.default_channel != self)):
            Server.remove_channel(self)    

    def join_channel(self, user):
        """Add a user to the channel list and tell everyone"""
        if ((user.channel != None) and (user.channel != self)):
            user.channel.leave_channel(user)
        user.channel = self
        self.userlist.append(user)
        self.broadcast((user.name + " has joined the channel."))
        user.update_chan_join_time()

    def find_earliest_user(self):
        """Returns the user that has the earliest channel join time"""
        user = min(self.userlist, key = attrgetter('chanJoinTime'))
        return user

    def get_topic(self):
        """Returns the channel topic"""
        return (self.name + " topic: " + self.topic + "\n")

    def set_topic(self, user, topic):
        """Sets the topic of the channel if the user is the owner"""
        if (self.owner == user):
            self.topic = topic
            self.broadcast((user.name + " changed the topic to: " + topic))
            return 0
        else:
            return -1


class ThreadHandler(threading.Thread):
    """Spins off threads for new clients to chat it up"""

    def __init__(self, client, address):
        """Constructor that sets client, address. name, and login time"""
        threading.Thread.__init__(self)
        self.client = client
        self.address = address
        self.userName = "Guest" + str(random.randint(0, 99999))
        self.conTime = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        NewUser = User(self.userName, self.client, self.conTime, self.address, Server.default_channel)
        self.user = NewUser
        Server.add_user(self.user)

    def initialize_chat_user(self):
        """Allow user to choose a username"""
        try:
            # Continue until a valid name is entered
            while True:
                self.client.send(("Please choose a username (between 2-" + str(LIMITS_USERNAME_LENGTH) + " characters):\n").encode())
                response = self.client.recv(4096)
                response = response.decode("utf-8").strip()
                response = response[0:LIMITS_USERNAME_LENGTH]
                response = response.replace(" ", "_")

                # Response must be at least 2 characters
                if (len(response) > 1):
                    user = Server.find_user(response)
                    # User must not exist
                    if (user == None):
                        self.userName = response
                        break
                    else:
                        self.client.send(("Username already taken. Try again.\n").encode())

        # User may break the username selection
        except UnicodeError:
            raise

        # Change username to selection
        self.user.change_username(self.userName)

        return response

    def chatLoop(self, response):
        """The loop for user input"""
        # Loop until the user chooses to /exit"
        while (response != "/exit"):
            try:
                response = self.user.client.recv(4096)
                response = response.decode("utf-8").strip()
                response = response[0:LIMITS_MESSAGE_LENGTH]

                # Empty message - did they quit or are they shy?
                if (len(response) == 0):
                    try:
                        self.user.client.send((" ").encode())
                    except socket.error:
                        raise
                # Potential command
                elif (response[0] == "/"):
                    command = (response.split(' ', 1)[0].lower())
                    # WHO command
                    if (command == "/who"):
                        # Who ALL
                        if (len(response.split(' ')) == 1): 
                            msg = Server.who_command()
                            self.user.client.send(msg.encode())
                        # Who CHANNEL
                        elif (response.split(' ', 2)[1][0] == "#"):
                            channelName = response.split(' ', 2)[1]
                            channel = Server.find_channel(channelName)
                            if (channel != None):
                                msg = self.user.channel.who_command()
                                self.user.client.send(msg.encode())
                            else:
                                self.user.client.send((channelName + " not found.\n").encode())
                        # Who USER
                        else:
                            userName = response.split(' ', 2)[1]
                            whoee = Server.find_user(userName)
                            if (whoee != None):
                                msg = whoee.who_command()
                                self.user.client.send(msg.encode())
                            else:
                                self.user.client.send((userName + " not found.\n").encode())
                    # MSG command
                    elif (command == "/msg"):
                        if (len(response.split(' ')) >= 3):
                            receiver = response.split(' ', 2)[1]
                            message = response[response.index(receiver) + len(receiver) + 1:]
                            Server.message_user(self.user, receiver, message)
                        else:
                            self.user.client.send(("Bad format. Try /msg user message\n").encode())
                    # USER command
                    elif (command == "/user"):
                        if (len(response.split(' ')) == 2):
                            self.user.change_username((response.split(' ', 2)[1])[0:LIMITS_USERNAME_LENGTH])
                        else:
                            self.user.client.send(("Bad format. Try /user newUser\n").encode())
                    # EXIT command
                    elif (command == "/exit"):
                        return
                    # HELP command
                    elif (command == "/help"):
                        msg = Server.help()
                        self.user.client.send((msg).encode())
                    # JOIN command
                    elif (command == "/join"):
                        if (len(response.split(' ')) > 1):
                            channelName = response.split(' ', 2)[1]
                            channel = Server.find_channel(channelName)
                            if (channel == None):
                                topic = "Default topic."
                                if (len(response.split(' ')) > 2):
                                    topic = response[response.index(channelName) + len(channelName) + 1:]
                                channel = Server.add_channel(channelName, topic, self.user)
                            chanMsg = channel.connection_message()
                            self.user.client.send(chanMsg.encode())
                            channel.join_channel(self.user)
                        else:
                            self.user.client.send(("Bad format. Try /join channel\n").encode())
                    # TOPIC command
                    elif (command == "/topic"):
                        if (len(response.split(' ')) > 1):
                            newTopic = response.split(' ', 1)[1]
                            if (self.user.channel.set_topic(self.user, newTopic) < 0):
                                self.user.client.send(("You must be the channel owner to set the topic.\n").encode())
                        else:
                            msg = self.user.channel.get_topic()
                            self.user.client.send((msg).encode())
                    # KICK command
                    elif (command == "/kick"):
                        if (len(response.split(' ')) > 1):
                            kickee = response.split(' ', 2)[1]
                            kickReturn = self.user.channel.kick_user(self.user, kickee)
                            if (kickReturn == -1):
                                self.user.client.send(("You must be the channel owner to kick someone.\n").encode())
                            elif (kickReturn == None):
                                self.user.client.send((kickee + " not found.\n").encode())
                        else:
                            self.user.client.send(("Bad format. Try /kick user\n").encode())
                    # Invalid command
                    else:
                        self.user.client.send(("Invalid command.\n").encode())
                # It's some chat!
                else:
                    self.user.channel.chat(self.user, response)
            except UnicodeError:
                self.user.client.send(("You tried to send invalid characters.\n").encode())
                raise

    def run(self):
        """The chat server starts here!"""
        try:
            print('{}: {} connected.'.format(datetime.datetime.now().time(), self.address))
            response = self.initialize_chat_user()

            # Print the initial message to newly connecting users
            helpMsg = Server.help()
            self.user.client.send(helpMsg.encode())
            # Print the default channel message
            chanMsg = self.user.channel.connection_message()
            self.user.client.send(chanMsg.encode())
            self.user.channel.join_channel(self.user)

            # Enter the chat loop; loop until done chatting
            self.chatLoop(response)

        except:
            """This just makes sure they don't break the server"""
            
        
        Server.kill_client(self.user)
        
        # Log the disconnect
        print('{}: {} disconnected.'.format(datetime.datetime.now().time(), self.user.address))



connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
connection.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
connection.bind(SERVER_ADDR)
connection.listen(10)

print('Chat server {} starting.'.format(Server.name))
print('Chat server address: ', format(SERVER_ADDR))
Server.add_channel("#Lobby", "Welcome to the lobby.", Server)

# Loop until the server is forcibly shut down
while True:
    client, address = connection.accept()

    thread = ThreadHandler(client, address)
    thread.start()

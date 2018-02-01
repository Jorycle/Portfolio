# Portfolio
Updated 01/21/2018

1. [Shell](#shell) (Unix, C/C++)
2. [Simple2d](#simple2d) (Windows, C++)
3. [Simple3d](#simple3d) (Windows, C++)
4. [Steganography](#steganography) (Windows, C++)
5. [Chatserv](#chatserv) (Python 3.6)

This is a compilation of some small projects I've completed. Each directory hosts the source code of a different project.
These programs were written in C++ using Visual Studio Community 2017 on Windows 10 unless otherwise noted.

## Shell
(Unix command line shell)

This is a simple UNIX shell that supports job control and a few basic commands.
It has a full command line interpreter that will correctly read quoted segments and break for break characters.
The shell supports piping and redirection.

**Basic commands:**
exit, cd, export, jobs, fg, bg, kill, help

Compile by running the makefile in a UNIX environment.

## Simple2d
(Windows-based GUI)

This is a simple line drawing program that draws two dimensional images using sets of line coordinates provided by a user-selected text file. An example Mario text file is included (Mario is missing his hands due to a tragic plumbing accident).

After loading an image file, the controls may be used to rotate, scale, or translate the image.
Basic transformations will use the coordinates (0,0) as the center of transformation.
Advanced transformations allow the user to provide the coordinates to transform about.
The current image can be saved using the "Save Lines" feature.

After each transformation, pressing the "Draw" button will update the image. This is done deliberately for academic purposes - under the hood, the rasterization uses matrices to manipulate and display the image. By updating only at draw, we can observe the power of matrix concatenation in action.

## Simple3d
(Windows-based GUI)

Similar to the 2d drawing program (see above), this program draws three dimensional images based on a slightly more complex data file provided by the user. An example of a basic cube has been included (the cube is also missing its hands).

The same operations in the 2d program are also present in the 3d program.
Additionally, users may set the coordinates, view orientation, and "up vector" of the camera using the text fields at the bottom right of the GUI.

## Steganography
(Windows-based GUI)

This is a simple tool that will embed data into a BMP format image file, or retrieve embedded data from an encoded BMP file.
The data is encoded into the *x* least significant bits in each pixel field, where *x* is a number (1-8) provided by the user.

***To encode  data into an image:***
1. Select the BMP image you would like to encode in the top file selector.
2. Select the data file you would like to encode it with in the bottom file selector.
3. Select the number of bits you would like to encode per pixel color field.
4. Press Encode, and save a new image file when prompted.

***To decode data from an image:***
1. Select the BMP image you would like to decode in the top file selector.
2. Select the same number of bits used when encoding the image.
3. Press decode, and save a new file when prompted.

## Chatserv
(Python 3.6, command line)

This is a simple Python 3.6 chat server. It was designed to be clientless so as to be the most portable, in the style of ye olde pre-web chat technology - as long as the server is running, anyone can telnet to the server with the host's IP and port 47629.

Among other features, notables include: Channels with topics; Channel owner-based moderation (kick, change topic); Channel logs; Query server, channel, and user information; Private messages between users.

The code makes use of a lot of basic software engineering practices. There is a lot of encapsulation and the coupling is very low. There is a lot of layering going on. This was done so that it would require very little modification if one decided to turn the chatserv into a client/server architecture and fully privatize the server-side functionality. In general, most features can be modified or added with little or no change to other code.

***To chat:***
1. Run the chatserv in a Python 3.6 environment.
2. Connect to the chatserv on the command line with telnet: telnet (host) 47629
3. Select a username
4. Start chatting! The introductory message or /help will tell you the commands.

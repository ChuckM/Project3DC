Project 3D Craft
=================

Welcome.

This is the Project 3D Craft Library (P3DC) project distribution.

The P3DC FAQ List (such as it is...)

This release is called : P3DC - Archive

Q) Did this used to be called "G3D" ?
A) Yes it did, that name was trademarked and this one isn't
   (yet :-) We're establishing a brand for it.

Q) What is the P3DC Library?

A) This library is my attempt at a 3D engine that is sufficiently
   flexible for me to "play around" with 3D games programming and
   3D experimentation. I've read for years the Graphics GEMs books
   (highly reccomemended) and been unable to "try out" the most of
   the suggested gems because I had no environment I knew well 
   enough to hack around it. I was also somewhat intimidated by the
   learning curve of Direct3D (and somewhat less so for OpenGL).

Q) Can I use this code in my Commercial Application?

A) Yes. The code is covered under a BSD style license. (it qualifies
   as "Open Source") See the file license.txt in the distribution.

Q) Why did you write this library?

A) So that I could speak intelligently about the issues of writing
   3D games.

   Long boring version :
   I started writing graphics programs in 1982 using a PDP-10 and a
   Tek 4010 storage scope, and a PDP-11/44 with a COMTAL video frame 
   buffer attached. Fun but the hardware required a air-conditioned 
   machine room. 

   Then in 1984 I bought an Amiga 1000 and had fun writing 
   programs for it (a lot cheaper but limited colors). 

   Between 1987 and 1995 I didn't write any graphics code. 

   In 1995 I bought an Edge3D board for my PC (used Nvidia NV1
   chip) but could never figure out how to program it! (Nvidia 
   was a pain to work with at the time and Direct3D version 1
   was a TOTAL joke)

   In 1996 I bought a Diamond Multimedia Monster 3d (Voodoo Graphics)
   based 3D accellerator for my (upgraded) PC. I tried to write a 
   Direct3D (V2) program and between the bugs in D3D the arcane 
   syntax/calls/overhead I gave up in disgust. Then I found the 3Dfx
   Glide library and when 2.4 was released I decided I'd see if I 
   could write a program to rotate a 3D cube. It took a while but
   by mid '97 I some code that could rotate a textured cube. Yippee!

   Of course this code could not do anything else :-(. So
   I bought the latest copy of Computer Graphics Principles and
   Practice, dusted off my old Neumann & Sproul Principles of
   Interactive Computer Graphics, and dug out my old ACM Siggraph
   papers and went to work. The result was "Chuck's Graphics Library"
   which could be used to render flat shaded objects in a 3D world
   with a fixed field of view camera. I rechristened the project
   the Project 3D Craft library (P3DC) in late '98 and rebuilt it twice
   more (versions 2, and 3) and got to the point where cameras 
   were getting flexible and I wanted to do animation. 

   Then in late '97 and early '98 I rewrote it a fourth time (P3DC-4)
   and got the polygon count up to 50-75K polygons a second, added
   textures and, with the help of my 7 yr old daughter, embarked 
   on making a 3D game. I ran into one roadblock after another and 
   decided I had the wrong architecture still, so I ripped it apart 
   and rewrote it again. That brings us up to the present with version
   5 of the library.

Q) How fast is this library?

A) That depends on many things, not the least of which are the 
   speed of the processor and the type of interface. The target
   for the library is 130K polygons/second on a P5-200 with
   a Voodoo1 card. That would give me a budget of approximately
   3K polygons per frame at 30FPS (We have to allow some time
   for game AI, sound, etc) But even this number is inaccurate
   because a scene could have 10K polygons and still work at
   30FPS if only 3K of those polygons were "visible" and ended
   up going down the pipeline to the display.

   The short answer is that I've got several game ideas cooking
   and it should be sufficient for all of them. None of them comes
   close to the complexity of Unreal :-). 

Q) What operating systems does it run on?

A) Windows95 and Linux. I support the Win95 version and <xxx> 
   supports the Linux version. When I can get the Glide libraries
   to work under FreeBSD there will be a FreeBSD version.

Q) Does it have to use Glide?

A) No. All of the actual "rendering" code goes through a "Graphics
   Abstraction Layer" (gal) that has to support what Glide supports
   however P3DC programs never "know" they are talking to Glide, they
   only know they are rendering graphics. I've written alternate
   GAL layers that generate PostScript(tm) for example. Ideally, someone
   will use some of Michael Abrash's excellent work and build a software
   scan converter back end that is generic to VGA or some such standard.
   You can also back end it using Direct3D or a subset of the OpenGL
   libraries.

Q) Why didn't you just use OpenGL?

A) Because I learn by building and I had to build a 3D engine to
   understand it. By using Glide I've got a pretty good device driver
   abstraction to work with. If you're just writing a game or
   something and either already know or don't care to know how
   3D engines work, I reccomend you use OpenGL.

Q) How long did it take you to write P3DC?

A) I've been working on it in my "spare" time for a couple of years
   now. However the bulk of the time spent on it was late 97 to mid-98. 
   Then I spent the middle of 98 thinking about how to re-write it and
   in August 1998 embarked on Version 5.

Q) Do you work on 3D graphics professionally?

A) No, I write and modify operating systems (generally UNIX based) and 
   design Internet appliances.

Q) I've modified P3DC to handle [some feature] can I distribute it?

A) Yes, any code you write you own and you can do whatever you want
   with it. However you are bound by the P3DC license to include my
   copyright statement where you put your own copyright statement
   (manuals, documentation, etc)

Q) Would you consider including my feature in the library?

A) Certainly, the beauty of open source is that anyone and everyone
   can contribute and make it better. I do retain change control on
   the final "product", anything can be placed in the "contrib" 
   directory.
   
RESOURCES

The Glide newsgroup on the 3DFX news server:
	news://news.3dfx.com/3dfx.glide

The comp.graphics.algorithms news group.

The comp.graphics.algorithms FAQ.

BOOKS

Computer Graphics : Principles and Practice (Systems Programming)
     by James D. Foley (Editor), Andries Van Dam (Contributor), 
        John Hughes (Contributor), van Dam, Feiner, Hughs 
     Addison-Wesley Pub Co; ISBN: 0201848406 

3D Computer Graphics
        by Alan Watt 
	Hardcover - 500 pages 2nd edition (September 1993) 
        Addison-Wesley Pub Co; ISBN: 0201631865

Graphics Gems
	by Andrew S. Glassner (Editor) 
	Hardcover Reissue edition (June 1993) 
        Academic Press; ISBN: 0122861663 

Graphics Gems II (The Graphics Gems)
	by James Arvo (Editor) 
	Hardcover Reissue edition (October 1994) 
	Academic Press; ISBN: 0120644819

Graphics Gems Iii/Book and IBM Disk
	by David Kirk (Editor) 
	Hardcover Bk&IBM dsk edition (January 1994) 
        Academic Press; ISBN: 0124096735

Graphics Gems Iv/Book and IBM Disk (The Graphics Gems Series)
        by Paul S. Heckbert (Editor) 
	Hardcover Book&Disk edition (May 1994) 
        Ap Professional; ISBN: 0123361559

Graphics Gems V/Book and IBM Version Disk (The Graphics Gems)
	by Alan W. Paeth (Editor) 
	Hardcover - 438 pages Book&Disk edition (May 1995) 
	Academic Press; ISBN: 0125434553


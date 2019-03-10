### About
Full Source of Esenthel Engine and its Tools (http://www.esenthel.com)<br/>
Official version available at: https://github.com/Esenthel/EsenthelEngine<br/>
Esenthel needs your support:
* Please donate: http://www.esenthel.com/?id=store&cat=1
* Or submit source code patches.


### License
<details><summary>Click to show</summary>

```
COPYRIGHT
Esenthel Engine created by Grzegorz Slazinski, all rights reserved.

ABOUT
Esenthel Engine was created mainly for purpose of making games, however it is perfectly suited for making
regular applications as well.
This License is non-exclusive, worldwide and royalty-free - you don't have to share the income that you make
from your games/apps made with Esenthel Engine.
You can create unlimited number of games/apps using Esenthel Engine.

LIMITATIONS
Esenthel Engine code/algorithms/designs may NOT be used for development/improvement of other Game Engines.
You may NOT browse Esenthel Engine source code if you work on improving other Game Engines, in that case
you may only compile it with the included tool and work with the compiled binary version of Esenthel Engine.

Esenthel Engine may NOT be used for - the purpose of/promoting/advertising/selling/assisting in/research/
development/production/improvement of: terrorism, illegal drugs, smoking, stealing, piracy, gambling, extortion,
fraud, spam, bullying, harming life (people and animals), polluting environment.

You may NOT resell or sub-license the source code to any third-party.
You may NOT claim that you wrote the source code.
You may NOT remove any copyright messages and this License text from the source code.
Esenthel source code is not sold to you, it is licensed to you, Esenthel Creators remain as its copyright owners.

If you submit any code to Esenthel Engine developers (for example bug fixes or improvements to the Engine) then
you agree that: Esenthel Engine developers have the right to use that code without any restrictions and without
paying you any fees/royalties for usage of that code. For example Esenthel Engine developers can modify that code
and incorporate it into the Engine, which then they can license to any third-party (which may include providing
source code access to the Engine, including codes submitted by you). In such case Esenthel Engine developers
do not owe you any payment for usage of that code.

Esenthel Engine authors are allowed to publicly share that you are using Esenthel Engine,
include your organization's logo in the information, and share your application's screenshots and videos
(including trailers, teasers, and gameplay).

TERMINATION
If you violate any terms of this agreement, or you are rude to/insult/harass Esenthel Engine authors or their
family, or you spread false information about the Engine or its authors, or you engage in any patent litigation
against Engine authors, then this License and access to Engine files will be terminated.

NO WARRANTY
This License does not include support or warranty of any kind - This software is provided 'as-is',
without any express or implied warranty. In no event will the authors be held liable
for any damages arising from the use of this software.

THIRD PARTY LIBRARIES
Esenthel Engine uses many third-party libraries located in "ThirdPartyLibs" folder, majority are completely
free to use, except the following which have some requirements:
Fraunhofer FDK AAC Sound Codec - please read its license carefully, it uses patented technology,
do not use the AAC sound codec unless you have a patent license.
Thank you to all of the third-party library developers!
```
</details>


### Folder Hierarchy

* **Data** - This entire folder gets converted into "Editor/Bin/Engine.pak" file (files required by the Engine to start)
* **Editor** - Esenthel Editor (some files for it need to be generated using the "Esenthel Builder" tool)
* **Editor Data** - This entire folder gets converted into "Editor/Bin/Editor.pak" file (files required by the Editor to start)
* **Editor Source** - Source code of Esenthel Editor
   * **\_Build\_** - Editor Source exported into C++ Project (Windows-Visual Studio, Mac-Xcode, Linux-NetBeans)
   * **0btc4f^3zje60097ikj149u0** - Tools as Esenthel Project to be opened in Esenthel Editor
   * **h3kv^1fvcwe4ri0#ll#761m7** - Editor Source as Esenthel Project to be opened in Esenthel Editor
   * **nop2ha8t22e6eg2!ck1odnk5** - Default Esenthel Gui Skins to be opened in Esenthel Editor
* **Engine** - Esenthel Engine (Headers, Source and C++ Projects)
* **Project** - Sample C++ Project (Windows-Visual Studio, Mac-Xcode, Linux-NetBeans) which can be used directly with the "Engine" without the need of "Editor"
* **ThirdPartyLibs** - All of the Third Party Libraries that Esenthel is based on (most of them include source and all have pre-compiled binaries that are ready to use)
* **Esenthel Builder** - Tool used to "Build" the Engine (compile the sources, link libraries, generate cleaned headers, generate "Engine.pak" from "Data" folder, generate "Code Editor.dat", etc.)
* **Esenthel.sln** - C++ Project in Windows Visual Studio format, that includes "Engine" and "Project" C++ Projects


### Getting things running
1. Download the entire repository to your computer
2. Start the "Esenthel Builder" tool and click "Do Selected" to run all checked tasks in order (compile the engine, link the libraries, compile the editor, etc.)
3. Enjoy!


### Esenthel Builder
It is important to run Esenthel Builder with all of the tasks that are checked by default, each time you update your engine source.

Simply start "Esenthel Builder" and click the "Do Selected" button at the bottom, which will execute all tasks that are currently selected.

Checked tasks will be executed in order from top to bottom, as some of the tasks depend on other tasks being executed first - those at the bottom may require that those at the top were already executed.


### Editions
Esenthel Engine is available in 2 Editions:
* **Binary** available from Esenthel Website
* **Source** available from GitHub Website

Sometimes Source Edition is updated more frequently, which means that the Binary Edition may be older.

Everytime when the Editor is started, it checks Esenthel Website for an update to the Engine, it checks the Binary Version.

Since the version on Esenthel Website will usually be different from the one that you compile manually from the Source Code, it will display that "an Update is available".

However if your Source version is newer than the Binary version on Esenthel Website, then after clicking "Update" you will be actually updating to an older version.

                                                                                                                                                                         
### Branches
There are 2 branches: Master and Development.

Master is the one you should be using, as Development is the branch in development, which may fail to compile, contain bugs or even break your project.
Do not use Development.

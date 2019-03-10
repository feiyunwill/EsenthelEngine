#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Auto.o \
	${OBJECTDIR}/Editor\ Update.o \
	${OBJECTDIR}/Animation.o \
	${OBJECTDIR}/Code@0.o \
	${OBJECTDIR}/Enum@0.o \
	${OBJECTDIR}/Environment@0.o \
	${OBJECTDIR}/Font@0.o \
	${OBJECTDIR}/Gui@1.o \
	${OBJECTDIR}/Gui\ Skin@0.o \
	${OBJECTDIR}/Icon@0.o \
	${OBJECTDIR}/Image.o \
	${OBJECTDIR}/Image\ Atlas.o \
	${OBJECTDIR}/Material@0.o \
	${OBJECTDIR}/Mini\ Map.o \
	${OBJECTDIR}/Adjust\ Bone\ Orientations.o \
	${OBJECTDIR}/AO.o \
	${OBJECTDIR}/Groups.o \
	${OBJECTDIR}/Leaf.o \
	${OBJECTDIR}/Lod.o \
	${OBJECTDIR}/Mesh.o \
	${OBJECTDIR}/Object@1.o \
	${OBJECTDIR}/Parts.o \
	${OBJECTDIR}/Transform@0.o \
	${OBJECTDIR}/Variations.o \
	${OBJECTDIR}/Panel@0.o \
	${OBJECTDIR}/Panel\ Image@0.o \
	${OBJECTDIR}/Param.o \
	${OBJECTDIR}/Phys\ Material@0.o \
	${OBJECTDIR}/Sound.o \
	${OBJECTDIR}/Text\ Style@0.o \
	${OBJECTDIR}/Video.o \
	${OBJECTDIR}/Water\ Material@0.o \
	${OBJECTDIR}/Area@0.o \
	${OBJECTDIR}/Brush@0.o \
	${OBJECTDIR}/Brush\ Color.o \
	${OBJECTDIR}/Brush\ Height.o \
	${OBJECTDIR}/Brush\ Material.o \
	${OBJECTDIR}/Builder.o \
	${OBJECTDIR}/Cursor.o \
	${OBJECTDIR}/Gui@0.o \
	${OBJECTDIR}/Import\ Terrain.o \
	${OBJECTDIR}/Object@0.o \
	${OBJECTDIR}/Object\ List.o \
	${OBJECTDIR}/Object\ Paint.o \
	${OBJECTDIR}/Selection.o \
	${OBJECTDIR}/Transform.o \
	${OBJECTDIR}/Undo.o \
	${OBJECTDIR}/Water@0.o \
	${OBJECTDIR}/Waypoint@0.o \
	${OBJECTDIR}/Waypoint\ List.o \
	${OBJECTDIR}/World.o \
	${OBJECTDIR}/Elm@0.o \
	${OBJECTDIR}/Controls.o \
	${OBJECTDIR}/Game.o \
	${OBJECTDIR}/Player.o \
	${OBJECTDIR}/Brush.o \
	${OBJECTDIR}/Calculator.o \
	${OBJECTDIR}/Compare\ Projects.o \
	${OBJECTDIR}/Convert\ To\ Atlas.o \
	${OBJECTDIR}/Convert\ To\ De-Atlas.o \
	${OBJECTDIR}/Copy\ Elements.o \
	${OBJECTDIR}/Create\ Materials.o \
	${OBJECTDIR}/Detect\ Similar\ Textures.o \
	${OBJECTDIR}/Export\ Element.o \
	${OBJECTDIR}/Gui.o \
	${OBJECTDIR}/Import\ 1.o \
	${OBJECTDIR}/Merge\ Similar\ Materials.o \
	${OBJECTDIR}/Misc\ Region.o \
	${OBJECTDIR}/Project\ Settings.o \
	${OBJECTDIR}/Project\ Update@0.o \
	${OBJECTDIR}/Size\ Statistics.o \
	${OBJECTDIR}/Split\ Animations.o \
	${OBJECTDIR}/Texture\ Downsize.o \
	${OBJECTDIR}/Theater.o \
	${OBJECTDIR}/Video\ Options.o \
	${OBJECTDIR}/Viewport4\ Region.o \
	${OBJECTDIR}/Import.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/Preview.o \
	${OBJECTDIR}/Project@0.o \
	${OBJECTDIR}/Project\ List.o \
	${OBJECTDIR}/Publish.o \
	${OBJECTDIR}/Server.o \
	${OBJECTDIR}/Server\ Editor.o \
	${OBJECTDIR}/Settings.o \
	${OBJECTDIR}/Area.o \
	${OBJECTDIR}/Commands.o \
	${OBJECTDIR}/Constants.o \
	${OBJECTDIR}/Code.o \
	${OBJECTDIR}/Enum.o \
	${OBJECTDIR}/Environment.o \
	${OBJECTDIR}/Font.o \
	${OBJECTDIR}/Gui\ Skin.o \
	${OBJECTDIR}/Icon.o \
	${OBJECTDIR}/Material.o \
	${OBJECTDIR}/Object.o \
	${OBJECTDIR}/Panel.o \
	${OBJECTDIR}/Panel\ Image.o \
	${OBJECTDIR}/Params.o \
	${OBJECTDIR}/Phys\ Material.o \
	${OBJECTDIR}/Skeleton.o \
	${OBJECTDIR}/Text\ Style.o \
	${OBJECTDIR}/Water.o \
	${OBJECTDIR}/Water\ Material.o \
	${OBJECTDIR}/Elm.o \
	${OBJECTDIR}/Functions.o \
	${OBJECTDIR}/Project.o \
	${OBJECTDIR}/Project\ Update.o \
	${OBJECTDIR}/Waypoint.o \
	${OBJECTDIR}/Internet\ Cache.o \
	${OBJECTDIR}/License\ Test.o \
	${OBJECTDIR}/Store.o \
	${OBJECTDIR}/Synchronize.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64 -fshort-wchar -fpermissive -ffast-math -ffriend-injection -Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-null-dereference -Wno-empty-body -Wno-address-of-temporary -include stdafx.h
CXXFLAGS=-m64 -fshort-wchar -fpermissive -ffast-math -ffriend-injection -Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-null-dereference -Wno-empty-body -Wno-address-of-temporary -include stdafx.h

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../../Editor/Bin/EsenthelEngine.a  -Wl,-rpath,Bin -lpthread -ldl -lX11 -lXi -lXinerama -lXrandr -lrt -lXmu -lGL -lopenal -lz -lodbc -ludev -lXcursor -lXxf86vm

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk Esenthel

Esenthel: ../../../Editor/Bin/EsenthelEngine.a 

Esenthel: ${OBJECTFILES}
	clang++ -o Esenthel ${OBJECTFILES} ${LDLIBSOPTIONS} -static-libstdc++

${OBJECTDIR}/Auto.o: Source/Auto.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Auto.o Source/Auto.cpp

${OBJECTDIR}/Editor\ Update.o: Source/Editor\ Update.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Editor\ Update.o Source/Editor\ Update.cpp

${OBJECTDIR}/Animation.o: Source/Editors/Animation.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Animation.o Source/Editors/Animation.cpp

${OBJECTDIR}/Code@0.o: Source/Editors/Code@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Code@0.o Source/Editors/Code@0.cpp

${OBJECTDIR}/Enum@0.o: Source/Editors/Enum@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Enum@0.o Source/Editors/Enum@0.cpp

${OBJECTDIR}/Environment@0.o: Source/Editors/Environment@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Environment@0.o Source/Editors/Environment@0.cpp

${OBJECTDIR}/Font@0.o: Source/Editors/Font@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Font@0.o Source/Editors/Font@0.cpp

${OBJECTDIR}/Gui@1.o: Source/Editors/Gui@1.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Gui@1.o Source/Editors/Gui@1.cpp

${OBJECTDIR}/Gui\ Skin@0.o: Source/Editors/Gui\ Skin@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Gui\ Skin@0.o Source/Editors/Gui\ Skin@0.cpp

${OBJECTDIR}/Icon@0.o: Source/Editors/Icon@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Icon@0.o Source/Editors/Icon@0.cpp

${OBJECTDIR}/Image.o: Source/Editors/Image.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Image.o Source/Editors/Image.cpp

${OBJECTDIR}/Image\ Atlas.o: Source/Editors/Image\ Atlas.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Image\ Atlas.o Source/Editors/Image\ Atlas.cpp

${OBJECTDIR}/Material@0.o: Source/Editors/Material@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Material@0.o Source/Editors/Material@0.cpp

${OBJECTDIR}/Mini\ Map.o: Source/Editors/Mini\ Map.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Mini\ Map.o Source/Editors/Mini\ Map.cpp

${OBJECTDIR}/Adjust\ Bone\ Orientations.o: Source/Editors/Object/Adjust\ Bone\ Orientations.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Adjust\ Bone\ Orientations.o Source/Editors/Object/Adjust\ Bone\ Orientations.cpp

${OBJECTDIR}/AO.o: Source/Editors/Object/AO.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/AO.o Source/Editors/Object/AO.cpp

${OBJECTDIR}/Groups.o: Source/Editors/Object/Groups.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Groups.o Source/Editors/Object/Groups.cpp

${OBJECTDIR}/Leaf.o: Source/Editors/Object/Leaf.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Leaf.o Source/Editors/Object/Leaf.cpp

${OBJECTDIR}/Lod.o: Source/Editors/Object/Lod.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Lod.o Source/Editors/Object/Lod.cpp

${OBJECTDIR}/Mesh.o: Source/Editors/Object/Mesh.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Mesh.o Source/Editors/Object/Mesh.cpp

${OBJECTDIR}/Object@1.o: Source/Editors/Object/Object@1.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Object@1.o Source/Editors/Object/Object@1.cpp

${OBJECTDIR}/Parts.o: Source/Editors/Object/Parts.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Parts.o Source/Editors/Object/Parts.cpp

${OBJECTDIR}/Transform@0.o: Source/Editors/Object/Transform@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Transform@0.o Source/Editors/Object/Transform@0.cpp

${OBJECTDIR}/Variations.o: Source/Editors/Object/Variations.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Variations.o Source/Editors/Object/Variations.cpp

${OBJECTDIR}/Panel@0.o: Source/Editors/Panel@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Panel@0.o Source/Editors/Panel@0.cpp

${OBJECTDIR}/Panel\ Image@0.o: Source/Editors/Panel\ Image@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Panel\ Image@0.o Source/Editors/Panel\ Image@0.cpp

${OBJECTDIR}/Param.o: Source/Editors/Param.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Param.o Source/Editors/Param.cpp

${OBJECTDIR}/Phys\ Material@0.o: Source/Editors/Phys\ Material@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Phys\ Material@0.o Source/Editors/Phys\ Material@0.cpp

${OBJECTDIR}/Sound.o: Source/Editors/Sound.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Sound.o Source/Editors/Sound.cpp

${OBJECTDIR}/Text\ Style@0.o: Source/Editors/Text\ Style@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Text\ Style@0.o Source/Editors/Text\ Style@0.cpp

${OBJECTDIR}/Video.o: Source/Editors/Video.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Video.o Source/Editors/Video.cpp

${OBJECTDIR}/Water\ Material@0.o: Source/Editors/Water\ Material@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Water\ Material@0.o Source/Editors/Water\ Material@0.cpp

${OBJECTDIR}/Area@0.o: Source/Editors/World/Area@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Area@0.o Source/Editors/World/Area@0.cpp

${OBJECTDIR}/Brush@0.o: Source/Editors/World/Brush@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Brush@0.o Source/Editors/World/Brush@0.cpp

${OBJECTDIR}/Brush\ Color.o: Source/Editors/World/Brush\ Color.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Brush\ Color.o Source/Editors/World/Brush\ Color.cpp

${OBJECTDIR}/Brush\ Height.o: Source/Editors/World/Brush\ Height.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Brush\ Height.o Source/Editors/World/Brush\ Height.cpp

${OBJECTDIR}/Brush\ Material.o: Source/Editors/World/Brush\ Material.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Brush\ Material.o Source/Editors/World/Brush\ Material.cpp

${OBJECTDIR}/Builder.o: Source/Editors/World/Builder.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Builder.o Source/Editors/World/Builder.cpp

${OBJECTDIR}/Cursor.o: Source/Editors/World/Cursor.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Cursor.o Source/Editors/World/Cursor.cpp

${OBJECTDIR}/Gui@0.o: Source/Editors/World/Gui@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Gui@0.o Source/Editors/World/Gui@0.cpp

${OBJECTDIR}/Import\ Terrain.o: Source/Editors/World/Import\ Terrain.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Import\ Terrain.o Source/Editors/World/Import\ Terrain.cpp

${OBJECTDIR}/Object@0.o: Source/Editors/World/Object@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Object@0.o Source/Editors/World/Object@0.cpp

${OBJECTDIR}/Object\ List.o: Source/Editors/World/Object\ List.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Object\ List.o Source/Editors/World/Object\ List.cpp

${OBJECTDIR}/Object\ Paint.o: Source/Editors/World/Object\ Paint.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Object\ Paint.o Source/Editors/World/Object\ Paint.cpp

${OBJECTDIR}/Selection.o: Source/Editors/World/Selection.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Selection.o Source/Editors/World/Selection.cpp

${OBJECTDIR}/Transform.o: Source/Editors/World/Transform.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Transform.o Source/Editors/World/Transform.cpp

${OBJECTDIR}/Undo.o: Source/Editors/World/Undo.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Undo.o Source/Editors/World/Undo.cpp

${OBJECTDIR}/Water@0.o: Source/Editors/World/Water@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Water@0.o Source/Editors/World/Water@0.cpp

${OBJECTDIR}/Waypoint@0.o: Source/Editors/World/Waypoint@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Waypoint@0.o Source/Editors/World/Waypoint@0.cpp

${OBJECTDIR}/Waypoint\ List.o: Source/Editors/World/Waypoint\ List.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Waypoint\ List.o Source/Editors/World/Waypoint\ List.cpp

${OBJECTDIR}/World.o: Source/Editors/World/World.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/World.o Source/Editors/World/World.cpp

${OBJECTDIR}/Elm@0.o: Source/Elm@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Elm@0.o Source/Elm@0.cpp

${OBJECTDIR}/Controls.o: Source/Game/Controls.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Controls.o Source/Game/Controls.cpp

${OBJECTDIR}/Game.o: Source/Game/Game.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Game.o Source/Game/Game.cpp

${OBJECTDIR}/Player.o: Source/Game/Player.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Player.o Source/Game/Player.cpp

${OBJECTDIR}/Brush.o: Source/Gui/Brush.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Brush.o Source/Gui/Brush.cpp

${OBJECTDIR}/Calculator.o: Source/Gui/Calculator.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Calculator.o Source/Gui/Calculator.cpp

${OBJECTDIR}/Compare\ Projects.o: Source/Gui/Compare\ Projects.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Compare\ Projects.o Source/Gui/Compare\ Projects.cpp

${OBJECTDIR}/Convert\ To\ Atlas.o: Source/Gui/Convert\ To\ Atlas.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Convert\ To\ Atlas.o Source/Gui/Convert\ To\ Atlas.cpp

${OBJECTDIR}/Convert\ To\ De-Atlas.o: Source/Gui/Convert\ To\ De-Atlas.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Convert\ To\ De-Atlas.o Source/Gui/Convert\ To\ De-Atlas.cpp

${OBJECTDIR}/Copy\ Elements.o: Source/Gui/Copy\ Elements.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Copy\ Elements.o Source/Gui/Copy\ Elements.cpp

${OBJECTDIR}/Create\ Materials.o: Source/Gui/Create\ Materials.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Create\ Materials.o Source/Gui/Create\ Materials.cpp

${OBJECTDIR}/Detect\ Similar\ Textures.o: Source/Gui/Detect\ Similar\ Textures.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Detect\ Similar\ Textures.o Source/Gui/Detect\ Similar\ Textures.cpp

${OBJECTDIR}/Export\ Element.o: Source/Gui/Export\ Element.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Export\ Element.o Source/Gui/Export\ Element.cpp

${OBJECTDIR}/Gui.o: Source/Gui/Gui.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Gui.o Source/Gui/Gui.cpp

${OBJECTDIR}/Import\ 1.o: Source/Gui/Import\ 1.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Import\ 1.o Source/Gui/Import\ 1.cpp

${OBJECTDIR}/Merge\ Similar\ Materials.o: Source/Gui/Merge\ Similar\ Materials.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Merge\ Similar\ Materials.o Source/Gui/Merge\ Similar\ Materials.cpp

${OBJECTDIR}/Misc\ Region.o: Source/Gui/Misc\ Region.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Misc\ Region.o Source/Gui/Misc\ Region.cpp

${OBJECTDIR}/Project\ Settings.o: Source/Gui/Project\ Settings.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Project\ Settings.o Source/Gui/Project\ Settings.cpp

${OBJECTDIR}/Project\ Update@0.o: Source/Gui/Project\ Update@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Project\ Update@0.o Source/Gui/Project\ Update@0.cpp

${OBJECTDIR}/Size\ Statistics.o: Source/Gui/Size\ Statistics.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Size\ Statistics.o Source/Gui/Size\ Statistics.cpp

${OBJECTDIR}/Split\ Animations.o: Source/Gui/Split\ Animations.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Split\ Animations.o Source/Gui/Split\ Animations.cpp

${OBJECTDIR}/Texture\ Downsize.o: Source/Gui/Texture\ Downsize.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Texture\ Downsize.o Source/Gui/Texture\ Downsize.cpp

${OBJECTDIR}/Theater.o: Source/Gui/Theater.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Theater.o Source/Gui/Theater.cpp

${OBJECTDIR}/Video\ Options.o: Source/Gui/Video\ Options.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Video\ Options.o Source/Gui/Video\ Options.cpp

${OBJECTDIR}/Viewport4\ Region.o: Source/Gui/Viewport4\ Region.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Viewport4\ Region.o Source/Gui/Viewport4\ Region.cpp

${OBJECTDIR}/Import.o: Source/Import.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Import.o Source/Import.cpp

${OBJECTDIR}/Main.o: Source/Main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Source/Main.cpp

${OBJECTDIR}/Preview.o: Source/Preview.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Preview.o Source/Preview.cpp

${OBJECTDIR}/Project@0.o: Source/Project@0.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Project@0.o Source/Project@0.cpp

${OBJECTDIR}/Project\ List.o: Source/Project\ List.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Project\ List.o Source/Project\ List.cpp

${OBJECTDIR}/Publish.o: Source/Publish.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Publish.o Source/Publish.cpp

${OBJECTDIR}/Server.o: Source/Server.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server.o Source/Server.cpp

${OBJECTDIR}/Server\ Editor.o: Source/Server\ Editor.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Server\ Editor.o Source/Server\ Editor.cpp

${OBJECTDIR}/Settings.o: Source/Settings.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Settings.o Source/Settings.cpp

${OBJECTDIR}/Area.o: Source/Shared/Area.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Area.o Source/Shared/Area.cpp

${OBJECTDIR}/Commands.o: Source/Shared/Commands.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Commands.o Source/Shared/Commands.cpp

${OBJECTDIR}/Constants.o: Source/Shared/Constants.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Constants.o Source/Shared/Constants.cpp

${OBJECTDIR}/Code.o: Source/Shared/Elements/Code.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Code.o Source/Shared/Elements/Code.cpp

${OBJECTDIR}/Enum.o: Source/Shared/Elements/Enum.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Enum.o Source/Shared/Elements/Enum.cpp

${OBJECTDIR}/Environment.o: Source/Shared/Elements/Environment.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Environment.o Source/Shared/Elements/Environment.cpp

${OBJECTDIR}/Font.o: Source/Shared/Elements/Font.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Font.o Source/Shared/Elements/Font.cpp

${OBJECTDIR}/Gui\ Skin.o: Source/Shared/Elements/Gui\ Skin.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Gui\ Skin.o Source/Shared/Elements/Gui\ Skin.cpp

${OBJECTDIR}/Icon.o: Source/Shared/Elements/Icon.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Icon.o Source/Shared/Elements/Icon.cpp

${OBJECTDIR}/Material.o: Source/Shared/Elements/Material.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Material.o Source/Shared/Elements/Material.cpp

${OBJECTDIR}/Object.o: Source/Shared/Elements/Object.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Object.o Source/Shared/Elements/Object.cpp

${OBJECTDIR}/Panel.o: Source/Shared/Elements/Panel.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Panel.o Source/Shared/Elements/Panel.cpp

${OBJECTDIR}/Panel\ Image.o: Source/Shared/Elements/Panel\ Image.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Panel\ Image.o Source/Shared/Elements/Panel\ Image.cpp

${OBJECTDIR}/Params.o: Source/Shared/Elements/Params.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Params.o Source/Shared/Elements/Params.cpp

${OBJECTDIR}/Phys\ Material.o: Source/Shared/Elements/Phys\ Material.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Phys\ Material.o Source/Shared/Elements/Phys\ Material.cpp

${OBJECTDIR}/Skeleton.o: Source/Shared/Elements/Skeleton.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Skeleton.o Source/Shared/Elements/Skeleton.cpp

${OBJECTDIR}/Text\ Style.o: Source/Shared/Elements/Text\ Style.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Text\ Style.o Source/Shared/Elements/Text\ Style.cpp

${OBJECTDIR}/Water.o: Source/Shared/Elements/Water.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Water.o Source/Shared/Elements/Water.cpp

${OBJECTDIR}/Water\ Material.o: Source/Shared/Elements/Water\ Material.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Water\ Material.o Source/Shared/Elements/Water\ Material.cpp

${OBJECTDIR}/Elm.o: Source/Shared/Elm.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Elm.o Source/Shared/Elm.cpp

${OBJECTDIR}/Functions.o: Source/Shared/Functions.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Functions.o Source/Shared/Functions.cpp

${OBJECTDIR}/Project.o: Source/Shared/Project.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Project.o Source/Shared/Project.cpp

${OBJECTDIR}/Project\ Update.o: Source/Shared/Project\ Update.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Project\ Update.o Source/Shared/Project\ Update.cpp

${OBJECTDIR}/Waypoint.o: Source/Shared/Waypoint.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Waypoint.o Source/Shared/Waypoint.cpp

${OBJECTDIR}/Internet\ Cache.o: Source/Store/Internet\ Cache.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Internet\ Cache.o Source/Store/Internet\ Cache.cpp

${OBJECTDIR}/License\ Test.o: Source/Store/License\ Test.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/License\ Test.o Source/Store/License\ Test.cpp

${OBJECTDIR}/Store.o: Source/Store/Store.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Store.o Source/Store/Store.cpp

${OBJECTDIR}/Synchronize.o: Source/Synchronize.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG=1 -I.  -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Synchronize.o Source/Synchronize.cpp



stdafx.h.pch: stdafx.h
	${MKDIR} -p ..
	@echo Performing Custom Build Step
	clang++ -x c++-header stdafx.h -o stdafx.h.pch $(CXXFLAGS) -g -DDEBUG=1 -I.  -std=c++14

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} stdafx.h.pch
	${RM} Esenthel

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

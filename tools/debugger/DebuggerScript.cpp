/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision$ (Revision of last commit) 
 $Date$ (Date of last commit)
 $Author$ (Author of last commit)
 
******************************************************************************/

#include "precompiled_engine.h"
#pragma hdrstop

static bool versioned = RegisterVersionedFile("$Id$");

#include "DebuggerApp.h"
#include "DebuggerScript.h"
#include "../../game/script/Script_Program.h"
#include "../../ui/Window.h"
#include "../../ui/UserInterfaceLocal.h"

/*
================
rvDebuggerScript::rvDebuggerScript
================
*/
rvDebuggerScript::rvDebuggerScript ( void )
{
	mContents  = NULL;
	mProgram   = NULL;
	mInterface = NULL;
}

/*
================
rvDebuggerScript::~rvDebuggerScript
================
*/
rvDebuggerScript::~rvDebuggerScript ( void )
{
	Unload ( );
}

/*
================
rvDebuggerScript::Unload

Unload the script from memory
================
*/
void rvDebuggerScript::Unload ( void )
{
	delete[] mContents;
	
	if ( mInterface )
	{
		delete mInterface;
	}
	else
	{	
		delete mProgram;
	}
	
	mContents  = NULL;
	mProgram   = NULL;
	mInterface = NULL;
}	

/*
================
rvDebuggerScript::Load

Loads the debugger script and attempts to compile it using the method 
appropriate for the file being loaded.  If the script cant be compiled
the loading of the script fails
================
*/
bool rvDebuggerScript::Load ( const char* filename )
{
	void* buffer;
	int	  size;

	// Unload the script before reloading it
	Unload ( );

	// Cache the filename used to load the script
	mFilename = filename;
		
	// Read in the file
	size = fileSystem->ReadFile ( filename, &buffer, &mModifiedTime );	
	if ( buffer == NULL )
	{
		return false;
	}
	
	// Copy the buffer over
	mContents = new char [ size + 1 ];
	memcpy ( mContents, buffer, size );
	mContents[size] = 0;	
	
	// Cleanup
	fileSystem->FreeFile ( buffer );

	// Now compile the script so we can tell what a valid line is, etc..  If its 
	// a gui file then we need to parse it using the userinterface system rather
	// than the normal script compiler.
	try
	{
		// Parse the script using the script compiler
		mProgram = new idProgram;
		mProgram->BeginCompilation ( );
		mProgram->CompileFile ( SCRIPT_DEFAULT );
		
		//BSM Nerve: Loads a game specific main script file
		idStr gamedir = cvarSystem->GetCVarString( "fs_currentfm" );
		if(gamedir.Length() > 0) {

			idStr scriptFile = va("script/%s_main.script", gamedir.c_str());
			if(fileSystem->ReadFile(scriptFile.c_str(), NULL) > 0) {
				mProgram.CompileFile(scriptFile.c_str());
			}

		}
		
		// Make sure the file isnt already compiled before trying to compile it again
		for ( int f = mProgram->NumFilenames() - 1; f >= 0; f -- )
		{
			idStr qpath;
			qpath = fileSystem->OSPathToRelativePath ( mProgram->GetFilename ( f ) );
			qpath.BackSlashesToSlashes ( );
			if ( !qpath.Cmp ( filename ) )
			{
				break;
			}
		}
		
		if ( f < 0 )
		{
			mProgram->CompileText ( filename, mContents, false );
		}
		
		mProgram->FinishCompilation ( );
	}
	catch ( idException& )
	{
		// Failed to parse the script so fail to load the file
		delete mProgram;
		mProgram = NULL;
		delete[] mContents;
		mContents = NULL;
		
		// TODO: Should cache the error for the dialog box
		
		return false;
	}

	return true;
}

/*
================
rvDebuggerScript::Reload

Reload the contents of the script
================
*/
bool rvDebuggerScript::Reload ( void )
{	
	return Load ( mFilename );
}

/*
================
rvDebuggerScript::IsValidLine

Determines whether or not the given line number within the script is a valid line of code
================
*/
bool rvDebuggerScript::IsLineCode ( int linenumber )
{
	int i;
	
	assert ( mProgram );

	// Run through all the statements in the program and see if any match the
	// linenumber that we are checking.
	for ( i	= 0; i < mProgram->NumStatements ( ); i ++ )
	{
		if ( mProgram->GetStatement ( i ).linenumber == linenumber )
		{
			return true;
		}
	}
	
	return false;
}

/*
================
rvDebuggerScript::IsFileModified

Determines whether or not the file loaded for this script has been modified since
it was loaded.
================
*/
bool rvDebuggerScript::IsFileModified ( bool updateTime )
{
	ID_TIME_T	t;
	bool	result = false;		

	// Grab the filetime and shut the file down
	fileSystem->ReadFile ( mFilename, NULL, &t );
	
	// Has the file been modified?
	if ( t > mModifiedTime )
	{
		result = true;
	}

	// If updateTime is true then we will update the modified time
	// stored in the script with the files current modified time
	if ( updateTime )
	{
		mModifiedTime = t;
	}
	
	return result;	
}

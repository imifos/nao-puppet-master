/*
 *  NAO Puppet Master - Game Controller Client
 *  ------------------------------------------
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License can be found here: 
 *  http://www.gnu.org/licenses/.
 *
 *  Author: 
 *    Tasha CARL, 2012, http://lucubratory.eu
 */

#include "StdAfx.h"
	

#include "NaoCommand.h"

// Static member object initialisation

CNaoCommand::CCommand CNaoCommand::SAY("NAO:SAY:",200);
CNaoCommand::CCommand CNaoCommand::SHUTUP("NAO:SHUTUP",100);
CNaoCommand::CCommand CNaoCommand::SITDOWN("NAO:SITDOWN",5000);
CNaoCommand::CCommand CNaoCommand::STANDUP("NAO:STANDUP",10000);
CNaoCommand::CCommand CNaoCommand::SETSTIFFNESS("NAO:SETSTIFFNESS:",0);


// ====================================================================================


/**
 * Default constructor.
 */
CNaoCommand::CNaoCommand() {

    m_rawCommand=NULL;
}


/**
 * Destructor.
 */
CNaoCommand::~CNaoCommand(void) {
    if (m_rawCommand!=NULL)
        delete m_rawCommand;
}



/**
 * Returns the raw message data string or NULL if not defined. 
 * NULL is also returned when the command is not yet out of detention.
 */
const char* CNaoCommand::GetRaw() {
    return m_rawCommand;
}


/**
 * Sets the raw message. The method clones the string and the caller
 * must release the memory. Raw message must alsways be set using this
 * method.
 */
void CNaoCommand::SetRaw(const char* rawCmd) {
    int l=strlen(rawCmd)+5;
    m_rawCommand=new char[l+1];
    strcpy_s(m_rawCommand,l,rawCmd);
    strcat_s(m_rawCommand,l,"^");
}


/**
 * Returns TRUE if the command is empty or not yet out of detention.
 */
bool CNaoCommand::IsEmpty() {
    return m_rawCommand==NULL;
}


/**
 * Returns the (cloned) raw command converted into a CString.
 *
 * The method should only be called when the called is sure
 * that the current command isn't a binary command.
 * The returned string must be deleted by the called
 * (exception to the rule).
 */
CString* CNaoCommand::GetShortenString(int length) {
    
    if (IsEmpty()) return new CString(_T(""));

    CString* s=new CString(GetRaw());
    if (s->GetLength()>120) {
        CString* s2=new CString(s->Left(120)+_T("..."));   
        delete s;
        return s2;
        // We are returning a pointer, so it must be on the heap 
        // to stay in focus.
    }
    else {
        return s;
    }
}


/**
 * NAO says the passed text.
 */
void CNaoCommand::Say(TCHAR* text) {

    if (!SAY.Release()) return;

    if (text!=NULL && wcslen(text)>0) {

        size_t converted;
        int l=strlen(SAY.Cmd());
        int tot=30+l+_tcslen(text)*2;

        char *raw=new char[tot+50];
        strcpy_s(raw,tot,SAY.Cmd());
        wcstombs_s(&converted,raw+l,tot,text,tot);

        SetRaw(raw);   
        
        delete raw;
    }
}


/**
 * NAO stops talking.
 */
void CNaoCommand::Shutup() {
    if (!SHUTUP.Release()) return;
    SetRaw(SHUTUP.Cmd());   
}


/**
 * Starts NAO behaviour SITDOWN.
 */
void CNaoCommand::Sitdown() {
    if (!SITDOWN.Release()) return;
    SetRaw(SITDOWN.Cmd());   
}


/**
 * Starts NAO behaviour STANDUP.
 */
void CNaoCommand::Standup() {
    if (!STANDUP.Release()) return;
    SetRaw(STANDUP.Cmd());   
}


/**
 * Sets NAOs body stiffness to x percent.
 */
void CNaoCommand::SetBodyStiffness(int percent) {
    
    if (!SETSTIFFNESS.Release()) return;

    if (percent<0) percent=0;
    if (percent>100) percent=100;

    char buf[50];
    sprintf_s(buf,"%d",percent);
    
    int l=strlen(SETSTIFFNESS.Cmd());
    char *cmd=new char[l+50];
    strcpy_s(cmd,l+50,SETSTIFFNESS.Cmd());
    strcat_s(cmd,l+50,buf);
    
    SetRaw(cmd);   

    delete cmd;
}





// ====================================================================================


/**
 * Initialising constructor.
 */
CNaoCommand::CCommand::CCommand(char* commandString,DWORD detention) {
    strcpy_s(m_commandString,commandString);
    m_detentionTime=detention;
    m_lastRelease=0;
}


/**
 * Returns the command string.
 */
char* CNaoCommand::CCommand::Cmd() { 
    return m_commandString; 
}

/**
 * Tries to release the command. If the detention period isn't finished, the
 * method returns false and it's forbidden to fire the command.
 */
bool CNaoCommand::CCommand::Release() { 

    if (m_detentionTime==0)
        return true; // no-detention command, fail fast

    DWORD now=GetTickCount(); // the 49.7 days limit shouldn't be a problem here

    if (m_lastRelease+m_detentionTime<now) {
        m_lastRelease=now;
        return true;
    }
    return false;
}
    
      
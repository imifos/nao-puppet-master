/*
 *  NAO Puppet Master - Server
 *  --------------------------
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

#include <winsock2.h> // needs ws2_32.lib

#include <iostream>
#include <string>
#include <sstream>

#include <signal.h>

#include <alerror/alerror.h>
#include <alproxies/altexttospeechproxy.h>
#include <alproxies/almemoryproxy.h>
#include <alproxies/albehaviormanagerproxy.h>
#include <alproxies/almotionproxy.h>



// Proxies to remote NaqQI
std::string g_naoAddress;
AL::ALTextToSpeechProxy* g_alTTS=NULL;
AL::ALMemoryProxy* g_alMEM=NULL;
AL::ALBehaviorManagerProxy* g_alBEH=NULL;
AL::ALMotionProxy* g_alMOT;

#define NAO_COMMAND_MEMORY "nao-puma-command"
#define NAO_PARAM1_MEMORY "nao-puma-param1"
#define NAO_BEHAVIOR "nao-puppetmaster"




// Network setting
#define APPKEY "SAUERKRAUT"
#define MSGBUFFERSIZE 1024
#define SERVERPORT  30457

SOCKET g_sServer;
SOCKET g_sClient;
SOCKADDR_IN g_sinServer;
char g_msgbuffer[MSGBUFFERSIZE+1];
char g_command[MSGBUFFERSIZE+1];




// Prototypes
void sighandler(int sig);
bool connectNaoProxies();
void runNaoBehaviour(const std::string command);
bool setupNetworkConnection();
bool receivedMessage();
void handleClient();
void handleNaoAction(std::string cmd);
void setBodyStiffness(const std::string value);
void handleSingleCommand(char* cmd);




// Helper for type-safe string to number convertion
template <class T> bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&)) {
    std::istringstream iss(s);
    return !(iss >> f >> t).fail();
}








//----------------------------------------------------------------------------
// Entry point - initialisation, network setup, NAO set-up etc.
// 
int main(int argc, char* argv[])
{
    std::cout << "NAO Puppet Master - Server v00.01" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Info: www.lucubratory.eu/projects/nao-puppet-master" << std::endl << std::endl;
    std::cout << "Usage: " << argv[0] << " [Target NAO's IP or local network name (Bonjour)] " << std::endl << std::endl;
    
    if (argc!=2) {
        std::cerr << "No parameter specified, try 'nao.local'" << std::endl << std::endl;
        //g_naoAddress="192.168.1.25"; 
        //g_naoAddress="lucus.local";
        g_naoAddress="nao.local";
    }
    else {
        std::cout << "NAO address: " << argv[1] << std::endl << std::endl;
        g_naoAddress=argv[1];
    }

    signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
    
    // Connecting to NAO
    std::cout << "Connecting to NAO, please wait." << std::endl;
    if (!connectNaoProxies())
        exit(-1);

    // Set-up network
    std::cout << "Set-up network." << std::endl;
    if (!setupNetworkConnection())
        exit(-2);

    // Accept clients (one at a time) and work with them
    int length;
    while(true) {

        std::cout << std::endl << std::endl << "Waiting for client connection (CTRL+C to exit the application)..."<<std::endl;    

        length=sizeof(g_sinServer);
        g_sClient=accept(g_sServer,(LPSOCKADDR)&g_sinServer,&length);

        if (g_sClient==INVALID_SOCKET) {
            std::cerr << "Initiating the incoming client connection failed. " << std::endl;
            Sleep(3000); 
        } 
        else {
            // Get the application key to ensure we are working with an authorized client.
            // (Or an authorised user on telnet).
            if (receivedMessage()) {
                if (strcmp(g_msgbuffer,APPKEY)!=0) {
                    std::cerr << "Client not authorised. Force disconnect." << std::endl;
                    Sleep(3000);
                    closesocket(g_sClient);
                }
                else {
                    // Ready to rumble...
                    handleClient();
                }
            }
        }

    } // while(true)

    char ch;
	std::cout << "To exit, press a key then press enter: ";
	std::cin >> ch;
    exit(0);
}


//----------------------------------------------------------------------------
// Handles the dialogue with one single client.
// 
void handleClient() {

    std::cout << "Client connected. oGnaCgnouC!" << std::endl;

    bool recok;
    std::string cmd="";
    char *next_token;
    do {
        recok=receivedMessage();
        if (recok) {
            char* token=strtok_s(g_msgbuffer,"^",&next_token);
            while (token) {
                handleSingleCommand(token);
                token=strtok_s(NULL,"^",&next_token);
            }
        }
    }
    while(recok);
}



//----------------------------------------------------------------------------
// Handles one single command received over the wire.
//
void handleSingleCommand(char* cmd) {

    size_t pos;

    // std::cout << "handle:"<<cmd << std::endl; // debugging

    // Invokes a high level NAO command or a nao-puppetmaster behavior (sitdown, standup, ...)
    pos=strcspn(cmd,"NAO:");
    if (pos==0) {
        strcpy_s(g_command,cmd+pos+4);
        handleNaoAction(g_command);
        return;
    }
            
    // Invokes a local server/proxy command (name, ...)
    pos=strcspn(cmd,"SERVER:");
    if (pos==0) {
        strcpy_s(g_command,cmd+pos+7);
        std::cout << "  Received SERVER command:" << g_command << std::endl;
        return;
    }

    // Invokes a low level NAO (real time) command (move motor,...) 
    pos=strcspn(cmd,"NAORT:");
    if (pos==0) {
        strcpy_s(g_command,cmd+pos+6);
        return;
    }
        
    std::cout << "  Ignore unrecognised message:" << cmd << std::endl;
}



//----------------------------------------------------------------------------
// Handles high level NAO commands, either sent directly to NAO or activated
// via the nao-puppetmaster behavior.
//
void handleNaoAction(std::string cmd) {

    std::cout << "  Handle NAO command: [" << cmd << "]" << std::endl;

    int pos;

    try {
	    //
        pos=cmd.find("SAY:");
        if (pos!=-1) return (void)g_alTTS->post.say(cmd.substr(pos+4,-1));
        //
        pos=cmd.find("STANDUP");
        if (pos!=-1) return runNaoBehaviour("STANDUP");
        //
        pos=cmd.find("SITDOWN");
        if (pos!=-1) return runNaoBehaviour("SITDOWN");
        // 
        pos=cmd.find("UNKNOWN");
        if (pos!=-1) return runNaoBehaviour("UNKNOWN");
        //
        pos=cmd.find("SHUTUP");
        if (pos!=-1) return (void)g_alTTS->post.stopAll();
        //
        pos=cmd.find("SETSTIFFNESS:");
        if (pos!=-1) return setBodyStiffness(cmd.substr(pos+strlen("SETSTIFFNESS:"),-1));
              

        std::cout << "  Unknown, ignored!" << std::endl;
    }
    catch (const AL::ALError& e) {
        std::cerr << "Failed to execute NAO command! "<< cmd << std::endl << " Caught exception: " << e.what() << std::endl << std::endl;
    }
}




//=========================================================================================
// NAO
//=========================================================================================


//----------------------------------------------------------------------------
// Sets the body stiffness to the passed percentage value.
//
void setBodyStiffness(const std::string percentageValueString) {

    int value;
    if (!from_string<int>(value,percentageValueString,std::dec)) {
        std::cout << "    setBodyStiffness - failed to convert [" << percentageValueString << "] to int!" << std::endl;
        return;
    }
    
    // New stiffness
    float fvalue=value/100.0f;
    if (fvalue<0.0) fvalue=0.0; 
    if (fvalue>1.0) fvalue=1.0; 

    // Current stiffness
    std::vector<float> stiffnesses=g_alMOT->getStiffnesses("Body");
    float currentStiffnesses=stiffnesses.front();

    // Dynamic ramp time in function of the stiffness delta
    float actionTime=abs(currentStiffnesses-fvalue)*0.7+0.2; 

    std::cout << "    setBodyStiffness - Current value: " << currentStiffnesses << ", new value: " << fvalue << " (" << value << "%) in " << actionTime << " seconds." << std::endl;
    
    g_alMOT->stiffnessInterpolation("Body",fvalue,actionTime);
}



//----------------------------------------------------------------------------
// Runs one of the pupper-master behaviors.
//
void runNaoBehaviour(const std::string command) {
    g_alMEM->insertData(NAO_COMMAND_MEMORY,(const std::string&)command);
	g_alBEH->runBehavior(NAO_BEHAVIOR); // blocking call!
}


//----------------------------------------------------------------------------
// Connect proxies to naoQI on the NAO.
//
bool connectNaoProxies() {

    try {
        g_alTTS=new AL::ALTextToSpeechProxy(g_naoAddress, 9559);
        g_alMEM=new AL::ALMemoryProxy(g_naoAddress, 9559);
	    g_alBEH=new AL::ALBehaviorManagerProxy(g_naoAddress, 9559);
	    g_alMOT=new AL::ALMotionProxy(g_naoAddress, 9559);

        const std::string phraseToSay = "Puppet Master Server connected. Resistance is futile.";
        g_alTTS->post.say(phraseToSay);
   
	    return true;
    }
    catch (const AL::ALError& e) {
        std::cerr << "Abort! Caught exception: " << e.what() << std::endl;
        return false;
    }
}




//=========================================================================================
// NETWORK
//=========================================================================================




//----------------------------------------------------------------------------
// Reads a message from the connected client. Returns either the number of 
// bytes read (>0), 0 if there is a graceful disconnection or SOCKET_ERROR (-1)
// in case of an error. The message is available in g_msgbuffer and g_msg. 
// In case of error, the client socket is closed.
// 
bool receivedMessage() {

    int len=recv(g_sClient,g_msgbuffer,MSGBUFFERSIZE,0);
    if (len<MSGBUFFERSIZE) 
        g_msgbuffer[len]=0; 
    else g_msgbuffer[MSGBUFFERSIZE]=0; // avoid buffer overflow
    
    if (len==SOCKET_ERROR) {
        std::cerr << "Error while receiving from client. Forced disconnect." << std::endl;
        closesocket(g_sClient);
        g_sClient=NULL;
        return false;
    }
    else if (len==0) {
        std::cout << "Client disconnected." << std::endl;
        closesocket(g_sClient);
        g_sClient=NULL;
        return false;
    }

    return true;
}


//----------------------------------------------------------------------------
// Prepares a SERVER socket on which we can listed for clients.
//
bool setupNetworkConnection() {

    WSADATA wsa; // WSADATA structure
    WORD vers; // version number
    
    memset(g_msgbuffer,0,MSGBUFFERSIZE+1);

    vers=MAKEWORD(2, 2);        // Set the version number to 2.2
    int err=WSAStartup(vers,&wsa);  // Start the WSADATA
    if (err!=0) {
        std::cerr << std::endl << "WSAStartup failed with error code " << err << std::endl << std::endl;
        WSACleanup();
        return false;
    }
    
    g_sServer=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_sServer==INVALID_SOCKET) {
        std::cerr << std::endl << "Socket creation failed with error code " << WSAGetLastError() << std::endl << std::endl;
        WSACleanup();
        return false;
    }
    
    memset(&g_sinServer, 0, sizeof(g_sinServer));
    g_sinServer.sin_family = AF_INET;
    g_sinServer.sin_addr.s_addr = INADDR_ANY; 
    g_sinServer.sin_port = htons(SERVERPORT); 

    if (bind(g_sServer,(LPSOCKADDR)&g_sinServer,sizeof(g_sinServer)) == SOCKET_ERROR) {
        std::cerr << std::endl << "Socket binding failed with error code " << WSAGetLastError() << std::endl << std::endl;
        WSACleanup();
        return false;
    }

    if (listen(g_sServer, 1 /* only one client */) == SOCKET_ERROR) {
        std::cerr << std::endl << "Socket listening failed with error code " << WSAGetLastError() << std::endl << std::endl;
        WSACleanup();
        return false;
    }

    return true;
}



//=========================================================================================
// SYSTEM
//=========================================================================================



//----------------------------------------------------------------------------
// Server app closed by signal: clean-up and leave.
//
void sighandler(int sig) {

    std::cout<< "Signal " << sig << " caught. Terminated server gracefully." << std::endl;
    
    closesocket(g_sClient);
    closesocket(g_sServer);
    WSACleanup();

    if (g_alTTS!=NULL) { g_alTTS->stopAll(); delete g_alTTS; }
    if (g_alMEM!=NULL) { delete g_alMEM; }
    if (g_alBEH!=NULL) { delete g_alBEH; }

    exit(0);
}











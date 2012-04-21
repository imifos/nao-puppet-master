#pragma once



class CNaoCommand {

private:

    class CCommand {
        private:
            char  m_commandString[50]; // raw command to send (excluding parameters)
            DWORD m_detentionTime;     // ms between 2 allowed releases of the command
            DWORD m_lastRelease;       // cpu ticker holding the moment of the last command release
        public:
            CCommand(char* commandString,DWORD detention);
            char* Cmd();
            bool Release();
    };

    static CCommand 
        SAY,SHUTUP,SITDOWN,STANDUP,SETSTIFFNESS;

    char* m_rawCommand; // Alawys use SetRaw() to set this value.

    void SetRaw(const char* rawCmd);

public:

    CNaoCommand(void);
    ~CNaoCommand(void);

    const char* GetRaw();
    CString* GetShortenString(int length);
    bool IsEmpty();

    void Say(TCHAR* text);
    void Shutup();
    void Sitdown();
    void Standup();
    void SetBodyStiffness(int percent);
};


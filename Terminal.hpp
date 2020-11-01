///
/// @author   GUSTAVO CAMPOS
/// @author   GUSTAVO CAMPOS
/// @date
/// @version  <#version#>
///
/// @copyright  (c) GUSTAVO CAMPOS, 2019
/// @copyright  Licence
///
/// @see    ReadMe.txt for references
///
//               GNU GENERAL PUBLIC LICENSE
//                Version 3, 29 June 2007
//
// Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
// Everyone is permitted to copy and distribute verbatim copies
// of this license document, but changing it is not allowed.
//
// Preamble
//
// The GNU General Public License is a free, copyleft license for
// software and other kinds of works.
//
// The licenses for most software and other practical works are designed
// to take away your freedom to share and change the works.  By contrast,
// the GNU General Public License is intended to guarantee your freedom to
// share and change all versions of a program--to make sure it remains free
// software for all its users.  We, the Free Software Foundation, use the
// GNU General Public License for most of our software; it applies also to
// any other work released this way by its authors.  You can apply it to
// your programs, too.
//
// See LICENSE file for the complete information

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <arduino.h>

#define TERM_VERSION "0"
#define TERM_SUBVERSION "1"
#define TERM_STAGE "dev"

uint8_t ParseOption (const String& commandLine, uint8_t nCommandIndex, String& returnText, bool count = false);

class TerminalStream
{
public:
    TerminalStream () = delete;

    TerminalStream (Stream& client);

    virtual bool IsConnected () = 0;
    virtual bool Disconnect () = 0;
    virtual void ContextSwitch () = 0;

    Stream& operator() ();

private:
    Stream& m_client;
};

class Terminal;

typedef void (*PrintFunction) (TerminalStream&);

class TerminalCommand
{
public:
    TerminalCommand ();

    virtual ~TerminalCommand ();

    virtual bool Execute (Terminal& terminal, TerminalStream& client , const String& strCommandLine) = 0;

    virtual void HelpMessage (TerminalStream& client) = 0;

private:
    TerminalStream* m_client;
    Stream* m_stream;
};

class Terminal
{
public:
    Terminal () = delete;

    Terminal (TerminalStream& stdio);

    ~Terminal ();

    void AttachMOTD (PrintFunction printFunction);

    void AttachPrompt (PrintFunction printFunction);

    void Start ();

    bool AttachCommand (const String& strCommandName, TerminalCommand& command);

    void PrintHelp ();

protected:
    bool ReadCommandLine (String& readCommand);

    bool WaitAvailableForReading ();

    bool ExecuteCommand (const String& commandLine);
    
    void PromptRedraw ();

    void CleanAllCommands ();

    TerminalCommand* GetCommand (const String& strCommand);
    
    struct CommandItem
    {
        CommandItem (TerminalCommand& command, String strCommandName, CommandItem* pNext);
        TerminalCommand& command;
        String strCommandName;
        CommandItem* pNext;
    };

    CommandItem* m_pStart;

private:
    String m_strCommandLine;
    TerminalStream& m_client;
    PrintFunction m_motdFunction;
    PrintFunction m_promptFunction;

    bool m_bStarted = false;
};

#endif
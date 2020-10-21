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

#include "Terminal.hpp"
#include <arduino.h>

#define TERMINAL_BS 8

/*
 * ---------------------------------
 * TerminalStream
 * ---------------------------------
 */

TerminalStream::TerminalStream (Stream& client) : m_client (client)
{
}

Stream& TerminalStream::operator() ()
{
    return m_client;
}

/*
 * ---------------------------------
 * TerminalCommand
 * ---------------------------------
 */

TerminalCommand::TerminalCommand ()
{
    m_client = (TerminalStream*)&Serial;
}

TerminalCommand::~TerminalCommand ()
{
}

void TerminalCommand::SetStdio (TerminalStream& stdio)
{
    m_client = &stdio;
}

TerminalStream& TerminalCommand::Stdio ()
{
    return *m_client;
}
/*
 * ---------------------------------
 * Terminal
 * ---------------------------------
 */

void GenericMOTDFunction (TerminalStream& stdio)
{
    stdio ().println ("---------------------------------");
    stdio ().println ("Embedded Terminal");
    stdio ().println (TERM_VERSION "." TERM_SUBVERSION "." TERM_STAGE);
    stdio ().println ("---------------------------------");
    stdio ().println ("");
}

void GenericPromptFunction (TerminalStream& stdio)
{
    stdio ().print (">");
}

Terminal::Terminal (TerminalStream& stdio) :
    m_strCommandLine (),
    m_client (stdio),
    m_motdFunction (GenericMOTDFunction),
    m_promptFunction (GenericPromptFunction),
    m_pStart (nullptr),
    m_bStarted (false)
{
}


Ternial::~Terminal ()
{
    CleanAllCommands ();
}

void Terminal::CleanAllCommands ()
{
    CommandItem* pCommandItem = nullptr; 
    if (m_pStart != nullptr)
    {
        
    }
}

void Terminal::AttachMOTD (PrintFunction printFunction)
{
    if (printFunction == nullptr)
    {
        exit (1);
    }

    m_motdFunction = printFunction;
}

void Terminal::AttachPrompt (PrintFunction printFunction)
{
    if (printFunction == nullptr)
    {
        exit (1);
    }

    m_promptFunction = printFunction;
}

Terminal::CommandItem::CommandItem (TerminalCommand& command, String strCommandName, CommandItem* pNext) :
    command (command), strCommandName (strCommandName), pNext (pNext)
{
}

bool Terminal::AttachCommand (const String& strCommandName, TerminalCommand& command)
{
    bool bRet = false;

    // Normalize command name
    String strCommand = strCommandName;
    strCommand.toUpperCase ();

    CommandItem* commandItem = new CommandItem (command, strCommand, m_pStart);

    if (commandItem != nullptr)
    {
        // Set Terminal's stdio
        command.SetStdio (m_client);

        // Will add using reverse order
        m_pStart = commandItem;
        bRet = true;
    }

    return bRet;
}

void Terminal::PrintHelp ()
{
    CommandItem* commandItem = nullptr;

    if (m_pStart != nullptr)
    {
        commandItem = m_pStart;
        m_client ().println ("Commands:");
        m_client ().println ("-------------------------");

        do
        {
            m_client ().print (commandItem->strCommandName);
            m_client ().print (":");
            commandItem->command.HelpMessage ();
        } while ((commandItem = commandItem->pNext) != nullptr);

        m_client ().println ("-------------------------");
    }
}

uint8_t Terminal::ParseOption (const String& commandLine, uint8_t nCommandIndex, String& returnText, bool countOnly)
{
    uint8_t nCommandOffSet = 0;

    nCommandIndex++;

    enum class state
    {
        NoText,
        Word,
        Text,
    } currentState;

    currentState = state::NoText;

    if (!countOnly)
    {
        returnText = "";
    }

    bool boolScape = false;

    String strBuffer = "";

    char chChar = 0;

    for (int nCount = 0; nCount < commandLine.length (); nCount++)
    {
        chChar = commandLine.charAt (nCount);

        if (currentState == state::NoText)
        {
            if (chChar == '"' || chChar == '\'')
            {
                // set Text state
                currentState = state::Text;
                nCommandOffSet++;
                continue;
            }
            else if (chChar != ' ')
            {
                // Set Word state
                currentState = state::Word;
                nCommandOffSet++;
            }
        }

        if (currentState != state::NoText)
        {
            if (boolScape == false)
            {
                if (currentState == state::Text && chChar == '"' || chChar == '\'')
                {
                    currentState = state::NoText;
                }
                else if (currentState == state::Word && chChar == ' ')
                {
                    currentState = state::NoText;
                }
                if (chChar == '\\')
                {
                    boolScape = true;
                    strBuffer = "";
                    continue;
                }
            }
            else if (!isdigit (chChar))
            {
                boolScape = false;
            }

            if (currentState == state::NoText)
            {
                if (countOnly == false && nCommandIndex == nCommandOffSet)
                {
                    break;
                }

                if (!countOnly) returnText = "";
            }
            else
            {
                if (countOnly == false)
                {
                    if (boolScape == true and isdigit (chChar))
                    {
                        strBuffer += chChar;
                    }
                    else
                    {
                        // To add special char \000\ 00 = number only
                        if (strBuffer.length () > 0)
                        {
                            returnText += static_cast<char> (atoi (strBuffer.c_str ()));
                            strBuffer = "";
                        }
                        else
                        {
                            returnText += static_cast<char> (chChar); /* code */
                        }
                    }
                }
            }
        }
    }

    if (!countOnly && (nCommandIndex != nCommandOffSet))
    {
        returnText = "";
        nCommandOffSet = 0;
    }
    else
    {
        nCommandOffSet--;
    }

    return nCommandOffSet;
}

bool Terminal::WaitAvailableForReading ()
{
    m_client.ContextSwitch ();

    do
    {
        if (m_client.IsConnected () == false)
        {
            return false;
        }

        m_client.ContextSwitch ();

    } while (m_client ().available () == 0);

    return true;
}

bool Terminal::ReadCommandLine (String& readCommand)
{
    uint8_t chChar = 0;

    readCommand = "";

    while (WaitAvailableForReading () == true)
    {
        m_client ().readBytes (&chChar, 1);
        // m_client ().print ("Read (");
        // m_client ().print ((int)chChar);
        // m_client ().print ("): [");
        // m_client ().print ((char) chChar >= 32 && chChar < 127 ? chChar : '.');
        // m_client ().println ("]");
        // m_client ().flush ();

        if (chChar == TERMINAL_BS && readCommand.length () > 0)
        {
            m_client ().print ((char)TERMINAL_BS);
            m_client ().print (' ');
            m_client ().print ((char)TERMINAL_BS);

            readCommand.remove (readCommand.length () - 1, 1);
        }
        else if (chChar == '\r')
        {
            m_client ().println ("");
            break;
        }
        else if (chChar >= 32 && chChar < 127)
        {
            m_client ().print ((char)chChar);
            readCommand += (char)chChar;
        }

        // m_client().print ("Read: ");
        // m_client().println (readCommand.c_str ());
    }

    if (m_client.IsConnected () == false)
    {
        return false;
    }

    return true;
}

void Terminal::PromptRedraw ()
{
    m_promptFunction (m_client);
    m_client ().print (m_strCommandLine);
}

void Terminal::Start ()
{
    m_motdFunction (m_client);

    PromptRedraw ();

    m_bStarted = true;

    m_client ().flush ();

    while (m_bStarted && ReadCommandLine (m_strCommandLine))
    {
        if (!ExecuteCommand (m_strCommandLine))
        {
            m_client ().println ("Error, command not recognized.");
        }

        PromptRedraw ();
    }

    m_client ().println ("Terminal finished-----------------------------");
}

TerminalCommand* Terminal::GetCommand (const String& strCommand)
{
    CommandItem* commandItem = nullptr;

    if (m_pStart != nullptr)
    {
        commandItem = m_pStart;

        do
        {
            if (commandItem->strCommandName == strCommand)
            {
                break;
            }
        } while ((commandItem = commandItem->pNext) != nullptr);

        if (commandItem == nullptr)
        {
            return nullptr;
        }
    }

    return &commandItem->command;
}

bool Terminal::ExecuteCommand (const String& commandLine)
{
    bool bRet = false;

    String strCommand = "";

    // m_client ().print ("Command: ");
    // m_client ().println (m_strCommandLine);

    // m_client ().print ("Itens: ");
    // m_client ().print (ParseOption (m_strCommandLine, 0xFF, strCommand, true));
    // m_client ().print (", Command: ");
    // m_client ().println (strCommand);

    ParseOption (m_strCommandLine, 0, strCommand, false);
    strCommand.toUpperCase ();

    if (strCommand == "EXIT")
    {
        m_bStarted = false;
        bRet = true;
    }
    else if (strCommand == "HELP")
    {
        PrintHelp ();
        bRet = true;
    }
    else
    {
        TerminalCommand* termCommand = nullptr;

        if ((termCommand = GetCommand (strCommand)) != nullptr)
        {
            if (!termCommand->Execute ())
            {
                m_client ().println ("Command error");
            }
            else
            {
                bRet = true;
            }
        }
    }

    m_strCommandLine = "";
    
    return bRet;
}

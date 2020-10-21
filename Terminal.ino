 ///
/// @author   GUSTAVO CAMPOS
/// @author   GUSTAVO CAMPOS
/// @date   28/05/2019 19:44
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

#include <arduino.h>
#include "Terminal.hpp"

class TStream : public TerminalStream
{
public:

    TStream(Stream& client) : TerminalStream (client)
    {
    }

    bool IsConnected () override
    {
        return true;
    }

    bool Disconnect () override
    {
        return true;
    }

    void ContextSwitch () override
    {
        return;
    }
};

TStream tstream = TStream (Serial);

class StatusCommand : public TerminalCommand
{
public:

    StatusCommand ()
    {}

    bool Execute ()
    {
        Stdio()().println ("Executing.... ");
        Stdio()().println ("done");

        return true;
    }

    void HelpMessage ()
    {
        Stdio()().println ("Show status of the terminal");
    }
};

StatusCommand statusCommand;

Terminal terminal (tstream);

void MOTDFunction (TerminalStream& stdio)
{
    stdio().println ("---------------------------------");
    stdio().println ("Embedded Terminal");
    stdio().println (TERM_VERSION "." TERM_SUBVERSION "." TERM_STAGE);
    stdio().println ("---------------------------------");
    terminal.PrintHelp ();
    stdio().println ("");
}

void setup()
{
    Serial.begin(115200);

    while (!Serial);

    Serial.println ("Terminal Demo");

    terminal.AttachMOTD (MOTDFunction);
    
    terminal.AttachCommand ("Status1", statusCommand);
    terminal.AttachCommand ("Status2", statusCommand);
    terminal.AttachCommand ("Status3", statusCommand);

    terminal.Start ();
}

void loop()
{

}

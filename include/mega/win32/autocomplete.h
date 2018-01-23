/**
* @file win32/autocomplete.h
* @brief Win32 console I/O autocomplete support
*
* (c) 2013-2018 by Mega Limited, Auckland, New Zealand
*
* This file is part of the MEGA SDK - Client Access Engine.
*
* Applications using the MEGA API must present a valid application key
* and comply with the the rules set forth in the Terms of Service.
*
* The MEGA SDK is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* @copyright Simplified (2-clause) BSD License.
*
* You should have received a copy of the license along with this
* program.
*/

#ifndef MEGA_AUTOCOMPLETE_H
#define MEGA_AUTOCOMPLETE_H 1

#include "mega/types.h"
#include <string>
#include <vector>
#include <memory>

namespace mega {
    class MEGA_API MegaClient;

namespace autocomplete {

    struct MEGA_API ACNode;
    typedef std::shared_ptr<ACNode> ACN;

    struct MEGA_API ACState
    {
        struct Completion {
            std::string s;
            bool caseInsensitive = false;
        };

        std::vector<Completion> completions;
        void addCompletion(const std::string& s, bool caseInsenstive = false);

        std::vector<std::pair<int, int>> wordPos;
        std::vector<std::string> words;
        unsigned i = 0;

        bool unixStyle = false;

        bool atCursor() {
            return i == words.size()-1;
        }
        const std::string& word() {
            return words[i];
        }
    };


    struct MEGA_API ACNode
    {
        // returns true if we should stop searching deeper than this node
        virtual bool addCompletions(ACState& s) = 0;

        // output suitable for user 'help'
        virtual std::ostream& describe(std::ostream& s) const = 0;
    };

    std::ostream& operator<<(std::ostream&, const ACNode&);

    struct MEGA_API Optional : public ACNode
    {
        ACN subnode;
        Optional(ACN n);
        bool isOptional();
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API Repeat : public ACNode
    {
        ACN subnode;
        Repeat(ACN n);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API Sequence : public ACNode
    {
        ACN current, next;
        Sequence(ACN n1, ACN n2);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API Text : public ACNode
    {
        std::string exactText;
        bool param;
        Text(const std::string& s, bool isParam);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API Flag : public ACNode
    {
        std::string flagText;
        Flag(const std::string& s);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API Either : public ACNode
    {
        std::vector<ACN> eithers;
        std::string describePrefix;
        Either(const std::string& describePrefix="");
        void Add(ACN n);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API WholeNumber : public ACNode
    {
        size_t defaultvalue;
        WholeNumber(size_t def_val);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API LocalFS : public ACNode
    {
        bool reportFiles = true;
        bool reportFolders = true;
        std::string descPref;
        LocalFS(bool files, bool folders, const std::string descriptionPrefix);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };

    struct MEGA_API MegaFS : public ACNode
    {
        MegaClient* client;
        ::mega::handle* cwd;
        bool reportFiles = true;
        bool reportFolders = true;
        std::string descPref;
        MegaFS(bool files, bool folders, MegaClient* a, ::mega::handle* curDirHandle, const std::string descriptionPrefix);
        bool addCompletions(ACState& s) override;
        std::ostream& describe(std::ostream& s) const override;
    };
    std::pair<int, int> identifyNextWord(const std::string& line, int startPos);

    struct MEGA_API CompletionState
    {
        std::string line;
        std::pair<int, int> wordPos;
        std::vector<ACState::Completion> completions;
        bool unixStyle = false;

        int lastAppliedIndex = -1;
        bool active = false;
        bool firstPressDone = false;
        size_t unixListCount = 0;
    };

    CompletionState autoComplete(const std::string line, size_t insertPos, ACN syntax, bool unixStyle);
    void applyCompletion(CompletionState& s, bool forwards, unsigned consoleWidth);

    // functions to bulid command descriptions
    ACN either(ACN n1 = nullptr, ACN n2 = nullptr, ACN n3 = nullptr, ACN n4 = nullptr);
    ACN sequence(ACN n1 = nullptr, ACN n2 = nullptr, ACN n3 = nullptr, ACN n4 = nullptr, ACN n5 = nullptr, ACN n6 = nullptr, ACN n7 = nullptr, ACN n8 = nullptr);
    ACN text(const std::string s);
    ACN param(const std::string s);
    ACN flag(const std::string s);
    ACN opt(ACN n);
    ACN repeat(ACN n);
    ACN wholenumber(size_t defaultvalue);
    ACN localFSPath(const std::string descriptionPrefix = "");
    ACN localFSFile(const std::string descriptionPrefix = "");
    ACN localFSFolder(const std::string descriptionPrefix = "");
    ACN remoteFSPath(MegaClient*, ::mega::handle*, const std::string descriptionPrefix = "");
    ACN remoteFSFile(MegaClient*, ::mega::handle*, const std::string descriptionPrefix = "");
    ACN remoteFSFolder(MegaClient*, ::mega::handle*, const std::string descriptionPrefix = "");

}}; //namespaces
#endif
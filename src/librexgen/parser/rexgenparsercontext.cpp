/*
    rexgen - a tool to create words based on regular expressions
    Copyright (C) 2012-2017  Jan Starke <jan.starke@outofbed.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
    more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin St, Fifth Floor, Boston, MA 02110, USA
*/


#include <librexgen/parser/rexgenparsercontext.h>
#include <algorithm>
#include <utility>
#include <cstring>
namespace rexgen {
  RexgenParserContext::RexgenParserContext(const char *input,
                                           const RexgenOptions &__options)
          : scanner(nullptr),
            result(nullptr),
            groupId(1),
            options(__options),
            streamRegex(nullptr) {
    InitScanner();
    assert(scanner != nullptr);

    int size;
    wchar_t wc = 0;
    mbstate_t mbs;
    std::memset(&mbs, 0, sizeof(mbs));
    do {
      size = mbrtowc(&wc, input, MB_CUR_MAX, &mbs);
      if (size > 0) {
        wcinput.push_back(wc);
        input += size;
      }
    } while (size > 0);
    next_char = wcinput.cbegin();
  }

/**
 * iterates through all group references and calls
 * updateGroupReferences for each
 */
  void RexgenParserContext::updateAllGroupReferences() {
    for (auto p : groups) {
      if (auto ptr = p.second.lock())
      updateGroupReferences(p.second);
    }
  }

  void RexgenParserContext::updateGroupReferences(const std::weak_ptr<Regex>& wre) {
    if (auto re = wre.lock()) {
      for (auto ref : groupRefs) {
        for (auto gr : (*ref.second)) {
          if (ref.first == re->getGroupId()) {
            gr->setRegex(re);
          }
        }
      }
    }
  }

  bool RexgenParserContext::hasInvalidGroupReferences() const {
    bool invalids = false;
    for (auto ref : groupRefs) {
      for (auto gr : *(ref.second)) {
        invalids |= (gr->getRegex().expired() == false);
      }
    }
    return invalids;
  }

  RexgenParserContext::~RexgenParserContext() {
    DestroyScanner();
  }

  void RexgenParserContext::registerGroupReference(std::shared_ptr<GroupReference> gr) {
    /* this is needed to later set the refered Regex */
    decltype(groupRefs)::iterator references = groupRefs.find(gr->getGroupId());
    if (references == groupRefs.end()) {
      groupRefs[gr->getGroupId()] = std::make_shared<std::set<std::shared_ptr<GroupReference> >>();
      references = groupRefs.find(gr->getGroupId());
    }
    (*references).second->insert(gr);
  }

  const std::shared_ptr<std::set<std::shared_ptr<GroupReference>>>& RexgenParserContext::getGroupReferences(
          int id) const {
    return groupRefs.at(id);
  }

  void RexgenParserContext::registerGroup(std::weak_ptr<Regex>& wre) {
    if (auto re = wre.lock()) {
      groups.insert(std::make_pair(re->getGroupId(),wre));
    }
  }

  const std::weak_ptr<Regex>& RexgenParserContext::getGroupRegex(int id) const {
    return groups.at(id);
  }

  const std::map<int, std::weak_ptr<Regex>> &RexgenParserContext::getGroups() const {
    return groups;
  }

  std::shared_ptr<Regex> RexgenParserContext::getStreamRegex() {
    if (streamRegex == nullptr) {
      streamRegex = std::make_shared<StreamRegex>(options.stream_callback);
      return streamRegex;
    } else {
      std::shared_ptr<GroupReference> gr = std::make_shared<GroupReference>(streamRegex->getId());
      gr->setRegex(streamRegex);
      return gr;
    }
  }

  wchar_t RexgenParserContext::getNextChar() {
    current_char = *next_char;
    ++next_char;
    return current_char;
  }
}
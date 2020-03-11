////////////////////////////////////////////////
// simCore/String
%apply double& OUTPUT { double& ang };
%include "simCore/String/Angle.h"

%include "simCore/String/Constants.h"
%include "simCore/String/FilePatterns.h"
%include "simCore/String/Format.h"

%apply size_t& OUTPUT { size_t& endWordPos };
%apply std::string& OUTPUT { std::string& tokenName, std::string& tokenValue };
%include "simCore/String/Tokenizer.h"

%include "simCore/String/Utils.h"

// Format.h
// TODO: join()
// TODO: getStrippedLine()

// Tokenizer.h
// TODO: stringTokenizer()
// TODO: getTokens()
// TODO: removeQuotes()
// TODO: tokenizeWithQuotes()
// TODO: quoteTokenizer()
// TODO: removeCommentTokens()
// TODO: quoteCommentTokenizer()
// TODO: escapeTokenize()

// TODO: Add these and test them as you add them
/*
%include "simCore/String/TextFormatter.h"
%include "simCore/String/TextReplacer.h"
%include "simCore/String/UnitContextFormatter.h"
%include "simCore/String/ValidNumber.h"
*/

#include <string>
#include <string.h>
#include <execinfo.h>
#include <zconf.h>
#include <link.h>
#include <limits>
#include <fcntl.h>

using std::string;

// Re-runs fn until it doesn't cause EINTR.
#define NO_INTR(fn)   do {} while ((fn) < 0 && errno == EINTR)

# define ATTRIBUTE_NOINLINE __attribute__ ((noinline))

#define GOOGLE_GLOG_DLL_DECL

#define DEFINE_VARIABLE(type, shorttype, name, value, meaning, tn)      \
  namespace fL##shorttype {                                             \
    GOOGLE_GLOG_DLL_DECL type FLAGS_##name(value);                      \
    char FLAGS_no##name;                                                \
  }                                                                     \
  using fL##shorttype::FLAGS_##name

#define EnvToBool(envname, dflt)   \
  (!getenv(envname) ? (dflt) : memchr("tTyY1\0", getenv(envname)[0], 6) != NULL)

#define DEFINE_bool(name, value, meaning) \
  DEFINE_VARIABLE(bool, B, name, value, meaning, bool)

#define SAFE_ASSERT(expr) ((expr) ? 0 : AssertFail())

#define GLOG_DEFINE_bool(name, value, meaning) \
  DEFINE_bool(name, EnvToBool("GLOG_" #name, value), meaning)

GLOG_DEFINE_bool(symbolize_stacktrace, true,
                 "Symbolize the stack trace in the tombstone");

// Installs a callback function, which will be called right before a symbol name
// is printed. The callback is intended to be used for showing a file name and a
// line number preceding a symbol name.
// "fd" is a file descriptor of the object file containing the program
// counter "pc". The callback function should write output to "out"
// and return the size of the output written. On error, the callback
// function should return -1.
typedef int (*SymbolizeCallback)(int fd,
                                 void *pc,
                                 char *out,
                                 size_t out_size,
                                 uint64_t relocation);

void InstallSymbolizeCallback(SymbolizeCallback callback);

static SymbolizeCallback g_symbolize_callback = NULL;

void InstallSymbolizeCallback(SymbolizeCallback callback)
{
    g_symbolize_callback = callback;
}

typedef void DebugWriter(const char *, void *);

// There is a better way, but this is good enough for our purpose.
# define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

// The %p field width for printf() functions is two characters per byte.
// For some environments, add two extra bytes for the leading "0x".
static const int kPrintfPointerFieldWidth = 2 + 2 * sizeof(void *);

// A wrapper for abort() to make it callable in ? :.
static int AssertFail()
{
    abort();
    return 0;  // Should not reach.
}

// Read up to "count" bytes from "offset" in the file pointed by file
// descriptor "fd" into the buffer starting at "buf" while handling short reads
// and EINTR.  On success, return the number of bytes read.  Otherwise, return
// -1.
static size_t ReadFromOffset(const int fd, void *buf, const long int count,
                              const off_t offset)
{
    SAFE_ASSERT(fd >= 0);
    SAFE_ASSERT(count <= std::numeric_limits<ssize_t>::max());
    char *buf0 = reinterpret_cast<char *>(buf);
    ssize_t num_bytes = 0;
    while (num_bytes < count)
    {
        ssize_t len;
        NO_INTR(len = pread(fd, buf0 + num_bytes, count - num_bytes,
                            offset + num_bytes));
        if (len < 0)
        {  // There was an error other than EINTR.
            return -1;
        }
        if (len == 0)
        {  // Reached EOF.
            break;
        }
        num_bytes += len;
    }
    SAFE_ASSERT(num_bytes <= count);
    return num_bytes;
}

// Place the hex number read from "start" into "*hex".  The pointer to
// the first non-hex character or "end" is returned.
static char *GetHex(const char *start, const char *end, uint64_t *hex)
{
    *hex = 0;
    const char *p;
    for (p = start; p < end; ++p)
    {
        int ch = *p;
        if ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
        {
            *hex = (*hex << 4) | (ch < 'A' ? ch - '0' : (ch & 0xF) + 9);
        }
        else
        {  // Encountered the first non-hex character.
            break;
        }
    }
    SAFE_ASSERT(p <= end);
    return const_cast<char *>(p);
}

typedef int (*SymbolizeOpenObjectFileCallback)(uint64_t pc,
                                               uint64_t &start_address,
                                               uint64_t &base_address,
                                               char *out_file_name,
                                               int out_file_name_size);

static SymbolizeOpenObjectFileCallback g_symbolize_open_object_file_callback =
        NULL;

void InstallSymbolizeOpenObjectFileCallback(
        SymbolizeOpenObjectFileCallback callback)
{
    g_symbolize_open_object_file_callback = callback;
}

// Thin wrapper around a file descriptor so that the file descriptor
// gets closed for sure.
struct FileDescriptor
{
    const int fd_;

    explicit FileDescriptor(int fd) : fd_(fd) {}

    ~FileDescriptor()
    {
        if (fd_ >= 0)
        {
            close(fd_);
        }
    }

    int get() { return fd_; }

private:
    explicit FileDescriptor(const FileDescriptor &);

    void operator=(const FileDescriptor &);
};


#if !defined(OS_WINDOWS)
typedef struct
{
    const char *abbrev;
    const char *real_name;
} AbbrevPair;

// Helper class for reading lines from file.
//
// Note: we don't use ProcMapsIterator since the object is big (it has
// a 5k array member) and uses async-unsafe functions such as sscanf()
// and snprintf().
class LineReader
{
public:
    explicit LineReader(int fd, char *buf, int buf_len, off_t offset)
            : fd_(fd),
              buf_(buf),
              buf_len_(buf_len),
              offset_(offset),
              bol_(buf),
              eol_(buf),
              eod_(buf) {}

    // Read '\n'-terminated line from file.  On success, modify "bol"
    // and "eol", then return true.  Otherwise, return false.
    //
    // Note: if the last line doesn't end with '\n', the line will be
    // dropped.  It's an intentional behavior to make the code simple.
    bool ReadLine(const char **bol, const char **eol)
    {
        if (BufferIsEmpty())
        {  // First time.
            const ssize_t num_bytes = ReadFromOffset(fd_, buf_, buf_len_, offset_);
            if (num_bytes <= 0)
            {  // EOF or error.
                return false;
            }
            offset_ += num_bytes;
            eod_ = buf_ + num_bytes;
            bol_ = buf_;
        }
        else
        {
            bol_ = eol_ + 1;  // Advance to the next line in the buffer.
            SAFE_ASSERT(bol_ <= eod_);  // "bol_" can point to "eod_".
            if (!HasCompleteLine())
            {
                const int incomplete_line_length = eod_ - bol_;
                // Move the trailing incomplete line to the beginning.
                memmove(buf_, bol_, incomplete_line_length);
                // Read text from file and append it.
                char *const append_pos = buf_ + incomplete_line_length;
                const int capacity_left = buf_len_ - incomplete_line_length;
                const ssize_t num_bytes =
                        ReadFromOffset(fd_, append_pos, capacity_left, offset_);
                if (num_bytes <= 0)
                {  // EOF or error.
                    return false;
                }
                offset_ += num_bytes;
                eod_ = append_pos + num_bytes;
                bol_ = buf_;
            }
        }
        eol_ = FindLineFeed();
        if (eol_ == NULL)
        {  // '\n' not found.  Malformed line.
            return false;
        }
        *eol_ = '\0';  // Replace '\n' with '\0'.

        *bol = bol_;
        *eol = eol_;
        return true;
    }

    // Beginning of line.
    const char *bol()
    {
        return bol_;
    }

    // End of line.
    const char *eol()
    {
        return eol_;
    }

private:
    explicit LineReader(const LineReader &);

    void operator=(const LineReader &);

    char *FindLineFeed()
    {
        return reinterpret_cast<char *>(memchr(bol_, '\n', eod_ - bol_));
    }

    bool BufferIsEmpty()
    {
        return buf_ == eod_;
    }

    bool HasCompleteLine()
    {
        return !BufferIsEmpty() && FindLineFeed() != NULL;
    }

    const int fd_;
    char *const buf_;
    const int buf_len_;
    off_t offset_;
    char *bol_;
    char *eol_;
    const char *eod_;  // End of data in "buf_".
};

// List of operators from Itanium C++ ABI.
static const AbbrevPair kOperatorList[] = {
        {"nw", "new"},
        {"na", "new[]"},
        {"dl", "delete"},
        {"da", "delete[]"},
        {"ps", "+"},
        {"ng", "-"},
        {"ad", "&"},
        {"de", "*"},
        {"co", "~"},
        {"pl", "+"},
        {"mi", "-"},
        {"ml", "*"},
        {"dv", "/"},
        {"rm", "%"},
        {"an", "&"},
        {"or", "|"},
        {"eo", "^"},
        {"aS", "="},
        {"pL", "+="},
        {"mI", "-="},
        {"mL", "*="},
        {"dV", "/="},
        {"rM", "%="},
        {"aN", "&="},
        {"oR", "|="},
        {"eO", "^="},
        {"ls", "<<"},
        {"rs", ">>"},
        {"lS", "<<="},
        {"rS", ">>="},
        {"eq", "=="},
        {"ne", "!="},
        {"lt", "<"},
        {"gt", ">"},
        {"le", "<="},
        {"ge", ">="},
        {"nt", "!"},
        {"aa", "&&"},
        {"oo", "||"},
        {"pp", "++"},
        {"mm", "--"},
        {"cm", ","},
        {"pm", "->*"},
        {"pt", "->"},
        {"cl", "()"},
        {"ix", "[]"},
        {"qu", "?"},
        {"st", "sizeof"},
        {"sz", "sizeof"},
        {NULL, NULL},
};

// List of builtin types from Itanium C++ ABI.
static const AbbrevPair kBuiltinTypeList[] = {
        {"v", "void"},
        {"w", "wchar_t"},
        {"b", "bool"},
        {"c", "char"},
        {"a", "signed char"},
        {"h", "unsigned char"},
        {"s", "short"},
        {"t", "unsigned short"},
        {"i", "int"},
        {"j", "unsigned int"},
        {"l", "long"},
        {"m", "unsigned long"},
        {"x", "long long"},
        {"y", "unsigned long long"},
        {"n", "__int128"},
        {"o", "unsigned __int128"},
        {"f", "float"},
        {"d", "double"},
        {"e", "long double"},
        {"g", "__float128"},
        {"z", "ellipsis"},
        {NULL, NULL}
};

// List of substitutions Itanium C++ ABI.
static const AbbrevPair kSubstitutionList[] = {
        {"St", ""},
        {"Sa", "allocator"},
        {"Sb", "basic_string"},
        // std::basic_string<char, std::char_traits<char>,std::allocator<char> >
        {"Ss", "string"},
        // std::basic_istream<char, std::char_traits<char> >
        {"Si", "istream"},
        // std::basic_ostream<char, std::char_traits<char> >
        {"So", "ostream"},
        // std::basic_iostream<char, std::char_traits<char> >
        {"Sd", "iostream"},
        {NULL, NULL}
};

// State needed for demangling.
typedef struct
{
    const char *mangled_cur;  // Cursor of mangled name.
    char *out_cur;            // Cursor of output string.
    const char *out_begin;    // Beginning of output string.
    const char *out_end;      // End of output string.
    const char *prev_name;    // For constructors/destructors.
    int prev_name_length;     // For constructors/destructors.
    short nest_level;         // For nested names.
    bool append;              // Append flag.
    bool overflowed;          // True if output gets overflowed.
} State;

// We don't use strlen() in libc since it's not guaranteed to be async
// signal safe.
static size_t StrLen(const char *str)
{
    size_t len = 0;
    while (*str != '\0')
    {
        ++str;
        ++len;
    }
    return len;
}

// Returns true if "str" has at least "n" characters remaining.
static bool AtLeastNumCharsRemaining(const char *str, int n)
{
    for (int i = 0; i < n; ++i)
    {
        if (str[i] == '\0')
        {
            return false;
        }
    }
    return true;
}

// Returns true if "str" has "prefix" as a prefix.
static bool StrPrefix(const char *str, const char *prefix)
{
    size_t i = 0;
    while (str[i] != '\0' && prefix[i] != '\0' &&
           str[i] == prefix[i])
    {
        ++i;
    }
    return prefix[i] == '\0';  // Consumed everything in "prefix".
}

static void InitState(State *state, const char *mangled,
                      char *out, int out_size)
{
    state->mangled_cur = mangled;
    state->out_cur = out;
    state->out_begin = out;
    state->out_end = out + out_size;
    state->prev_name = NULL;
    state->prev_name_length = -1;
    state->nest_level = -1;
    state->append = true;
    state->overflowed = false;
}

// Returns true and advances "mangled_cur" if we find "one_char_token"
// at "mangled_cur" position.  It is assumed that "one_char_token" does
// not contain '\0'.
static bool ParseOneCharToken(State *state, const char one_char_token)
{
    if (state->mangled_cur[0] == one_char_token)
    {
        ++state->mangled_cur;
        return true;
    }
    return false;
}

// Returns true and advances "mangled_cur" if we find "two_char_token"
// at "mangled_cur" position.  It is assumed that "two_char_token" does
// not contain '\0'.
static bool ParseTwoCharToken(State *state, const char *two_char_token)
{
    if (state->mangled_cur[0] == two_char_token[0] &&
        state->mangled_cur[1] == two_char_token[1])
    {
        state->mangled_cur += 2;
        return true;
    }
    return false;
}

// Returns true and advances "mangled_cur" if we find any character in
// "char_class" at "mangled_cur" position.
static bool ParseCharClass(State *state, const char *char_class)
{
    const char *p = char_class;
    for (; *p != '\0'; ++p)
    {
        if (state->mangled_cur[0] == *p)
        {
            ++state->mangled_cur;
            return true;
        }
    }
    return false;
}

// This function is used for handling an optional non-terminal.
static bool Optional(bool)
{
    return true;
}

// This function is used for handling <non-terminal>+ syntax.
typedef bool (*ParseFunc)(State *);

static bool OneOrMore(ParseFunc parse_func, State *state)
{
    if (parse_func(state))
    {
        while (parse_func(state))
        {
        }
        return true;
    }
    return false;
}

// This function is used for handling <non-terminal>* syntax. The function
// always returns true and must be followed by a termination token or a
// terminating sequence not handled by parse_func (e.g.
// ParseOneCharToken(state, 'E')).
static bool ZeroOrMore(ParseFunc parse_func, State *state)
{
    while (parse_func(state))
    {
    }
    return true;
}

// Append "str" at "out_cur".  If there is an overflow, "overflowed"
// is set to true for later use.  The output string is ensured to
// always terminate with '\0' as long as there is no overflow.
static void Append(State *state, const char *const str, const int length)
{
    int i;
    for (i = 0; i < length; ++i)
    {
        if (state->out_cur + 1 < state->out_end)
        {  // +1 for '\0'
            *state->out_cur = str[i];
            ++state->out_cur;
        }
        else
        {
            state->overflowed = true;
            break;
        }
    }
    if (!state->overflowed)
    {
        *state->out_cur = '\0';  // Terminate it with '\0'
    }
}

// We don't use equivalents in libc to avoid locale issues.
static bool IsLower(char c)
{
    return c >= 'a' && c <= 'z';
}

static bool IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

// Returns true if "str" is a function clone suffix.  These suffixes are used
// by GCC 4.5.x and later versions to indicate functions which have been
// cloned during optimization.  We treat any sequence (.<alpha>+.<digit>+)+ as
// a function clone suffix.
static bool IsFunctionCloneSuffix(const char *str)
{
    size_t i = 0;
    while (str[i] != '\0')
    {
        // Consume a single .<alpha>+.<digit>+ sequence.
        if (str[i] != '.' || !IsAlpha(str[i + 1]))
        {
            return false;
        }
        i += 2;
        while (IsAlpha(str[i]))
        {
            ++i;
        }
        if (str[i] != '.' || !IsDigit(str[i + 1]))
        {
            return false;
        }
        i += 2;
        while (IsDigit(str[i]))
        {
            ++i;
        }
    }
    return true;  // Consumed everything in "str".
}

// Append "str" with some tweaks, iff "append" state is true.
// Returns true so that it can be placed in "if" conditions.
static void MaybeAppendWithLength(State *state, const char *const str,
                                  const int length)
{
    if (state->append && length > 0)
    {
        // Append a space if the output buffer ends with '<' and "str"
        // starts with '<' to avoid <<<.
        if (str[0] == '<' && state->out_begin < state->out_cur &&
            state->out_cur[-1] == '<')
        {
            Append(state, " ", 1);
        }
        // Remember the last identifier name for ctors/dtors.
        if (IsAlpha(str[0]) || str[0] == '_')
        {
            state->prev_name = state->out_cur;
            state->prev_name_length = length;
        }
        Append(state, str, length);
    }
}

// A convenient wrapper arount MaybeAppendWithLength().
static bool MaybeAppend(State *state, const char *const str)
{
    if (state->append)
    {
        int length = StrLen(str);
        MaybeAppendWithLength(state, str, length);
    }
    return true;
}

// This function is used for handling nested names.
static bool EnterNestedName(State *state)
{
    state->nest_level = 0;
    return true;
}

// This function is used for handling nested names.
static bool LeaveNestedName(State *state, short prev_value)
{
    state->nest_level = prev_value;
    return true;
}

// Disable the append mode not to print function parameters, etc.
static bool DisableAppend(State *state)
{
    state->append = false;
    return true;
}

// Restore the append mode to the previous state.
static bool RestoreAppend(State *state, bool prev_value)
{
    state->append = prev_value;
    return true;
}

// Increase the nest level for nested names.
static void MaybeIncreaseNestLevel(State *state)
{
    if (state->nest_level > -1)
    {
        ++state->nest_level;
    }
}

// Appends :: for nested names if necessary.
static void MaybeAppendSeparator(State *state)
{
    if (state->nest_level >= 1)
    {
        MaybeAppend(state, "::");
    }
}

// Cancel the last separator if necessary.
static void MaybeCancelLastSeparator(State *state)
{
    if (state->nest_level >= 1 && state->append &&
        state->out_begin <= state->out_cur - 2)
    {
        state->out_cur -= 2;
        *state->out_cur = '\0';
    }
}

// Returns true if the identifier of the given length pointed to by
// "mangled_cur" is anonymous namespace.
static bool IdentifierIsAnonymousNamespace(State *state, int length)
{
    static const char anon_prefix[] = "_GLOBAL__N_";
    return (length > (int) sizeof(anon_prefix) - 1 &&  // Should be longer.
            StrPrefix(state->mangled_cur, anon_prefix));
}

// Forward declarations of our parsing functions.
static bool ParseMangledName(State *state);

static bool ParseEncoding(State *state);

static bool ParseName(State *state);

static bool ParseUnscopedName(State *state);

static bool ParseUnscopedTemplateName(State *state);

static bool ParseNestedName(State *state);

static bool ParsePrefix(State *state);

static bool ParseUnqualifiedName(State *state);

static bool ParseSourceName(State *state);

static bool ParseLocalSourceName(State *state);

static bool ParseNumber(State *state, int *number_out);

static bool ParseFloatNumber(State *state);

static bool ParseSeqId(State *state);

static bool ParseIdentifier(State *state, int length);

static bool ParseAbiTags(State *state);

static bool ParseAbiTag(State *state);

static bool ParseOperatorName(State *state);

static bool ParseSpecialName(State *state);

static bool ParseCallOffset(State *state);

static bool ParseNVOffset(State *state);

static bool ParseVOffset(State *state);

static bool ParseCtorDtorName(State *state);

static bool ParseType(State *state);

static bool ParseCVQualifiers(State *state);

static bool ParseBuiltinType(State *state);

static bool ParseFunctionType(State *state);

static bool ParseBareFunctionType(State *state);

static bool ParseClassEnumType(State *state);

static bool ParseArrayType(State *state);

static bool ParsePointerToMemberType(State *state);

static bool ParseTemplateParam(State *state);

static bool ParseTemplateTemplateParam(State *state);

static bool ParseTemplateArgs(State *state);

static bool ParseTemplateArg(State *state);

static bool ParseExpression(State *state);

static bool ParseExprPrimary(State *state);

static bool ParseLocalName(State *state);

static bool ParseDiscriminator(State *state);

static bool ParseSubstitution(State *state);

// Implementation note: the following code is a straightforward
// translation of the Itanium C++ ABI defined in BNF with a couple of
// exceptions.
//
// - Support GNU extensions not defined in the Itanium C++ ABI
// - <prefix> and <template-prefix> are combined to avoid infinite loop
// - Reorder patterns to shorten the code
// - Reorder patterns to give greedier functions precedence
//   We'll mark "Less greedy than" for these cases in the code
//
// Each parsing function changes the state and returns true on
// success.  Otherwise, don't change the state and returns false.  To
// ensure that the state isn't changed in the latter case, we save the
// original state before we call more than one parsing functions
// consecutively with &&, and restore the state if unsuccessful.  See
// ParseEncoding() as an example of this convention.  We follow the
// convention throughout the code.
//
// Originally we tried to do demangling without following the full ABI
// syntax but it turned out we needed to follow the full syntax to
// parse complicated cases like nested template arguments.  Note that
// implementing a full-fledged demangler isn't trivial (libiberty's
// cp-demangle.c has +4300 lines).
//
// Note that (foo) in <(foo) ...> is a modifier to be ignored.
//
// Reference:
// - Itanium C++ ABI
//   <http://www.codesourcery.com/cxx-abi/abi.html#mangling>

// <mangled-name> ::= _Z <encoding>
static bool ParseMangledName(State *state)
{
    return ParseTwoCharToken(state, "_Z") && ParseEncoding(state);
}

// <encoding> ::= <(function) name> <bare-function-type>
//            ::= <(data) name>
//            ::= <special-name>
static bool ParseEncoding(State *state)
{
    State copy = *state;
    if (ParseName(state) && ParseBareFunctionType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseName(state) || ParseSpecialName(state))
    {
        return true;
    }
    return false;
}

// <name> ::= <nested-name>
//        ::= <unscoped-template-name> <template-args>
//        ::= <unscoped-name>
//        ::= <local-name>
static bool ParseName(State *state)
{
    if (ParseNestedName(state) || ParseLocalName(state))
    {
        return true;
    }

    State copy = *state;
    if (ParseUnscopedTemplateName(state) &&
        ParseTemplateArgs(state))
    {
        return true;
    }
    *state = copy;

    // Less greedy than <unscoped-template-name> <template-args>.
    if (ParseUnscopedName(state))
    {
        return true;
    }
    return false;
}

// <unscoped-name> ::= <unqualified-name>
//                 ::= St <unqualified-name>
static bool ParseUnscopedName(State *state)
{
    if (ParseUnqualifiedName(state))
    {
        return true;
    }

    State copy = *state;
    if (ParseTwoCharToken(state, "St") &&
        MaybeAppend(state, "std::") &&
        ParseUnqualifiedName(state))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <unscoped-template-name> ::= <unscoped-name>
//                          ::= <substitution>
static bool ParseUnscopedTemplateName(State *state)
{
    return ParseUnscopedName(state) || ParseSubstitution(state);
}

// <nested-name> ::= N [<CV-qualifiers>] <prefix> <unqualified-name> E
//               ::= N [<CV-qualifiers>] <template-prefix> <template-args> E
static bool ParseNestedName(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'N') &&
        EnterNestedName(state) &&
        Optional(ParseCVQualifiers(state)) &&
        ParsePrefix(state) &&
        LeaveNestedName(state, copy.nest_level) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;
    return false;
}

// This part is tricky.  If we literally translate them to code, we'll
// end up infinite loop.  Hence we merge them to avoid the case.
//
// <prefix> ::= <prefix> <unqualified-name>
//          ::= <template-prefix> <template-args>
//          ::= <template-param>
//          ::= <substitution>
//          ::= # empty
// <template-prefix> ::= <prefix> <(template) unqualified-name>
//                   ::= <template-param>
//                   ::= <substitution>
static bool ParsePrefix(State *state)
{
    bool has_something = false;
    while (true)
    {
        MaybeAppendSeparator(state);
        if (ParseTemplateParam(state) ||
            ParseSubstitution(state) ||
            ParseUnscopedName(state))
        {
            has_something = true;
            MaybeIncreaseNestLevel(state);
            continue;
        }
        MaybeCancelLastSeparator(state);
        if (has_something && ParseTemplateArgs(state))
        {
            return ParsePrefix(state);
        }
        else
        {
            break;
        }
    }
    return true;
}

// <unqualified-name> ::= <operator-name>
//                    ::= <ctor-dtor-name>
//                    ::= <source-name> [<abi-tags>]
//                    ::= <local-source-name> [<abi-tags>]
static bool ParseUnqualifiedName(State *state)
{
    return (ParseOperatorName(state) ||
            ParseCtorDtorName(state) ||
            (ParseSourceName(state) && Optional(ParseAbiTags(state))) ||
            (ParseLocalSourceName(state) && Optional(ParseAbiTags(state))));
}

// <source-name> ::= <positive length number> <identifier>
static bool ParseSourceName(State *state)
{
    State copy = *state;
    int length = -1;
    if (ParseNumber(state, &length) && ParseIdentifier(state, length))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <local-source-name> ::= L <source-name> [<discriminator>]
//
// References:
//   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=31775
//   http://gcc.gnu.org/viewcvs?view=rev&revision=124467
static bool ParseLocalSourceName(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'L') && ParseSourceName(state) &&
        Optional(ParseDiscriminator(state)))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <number> ::= [n] <non-negative decimal integer>
// If "number_out" is non-null, then *number_out is set to the value of the
// parsed number on success.
static bool ParseNumber(State *state, int *number_out)
{
    int sign = 1;
    if (ParseOneCharToken(state, 'n'))
    {
        sign = -1;
    }
    const char *p = state->mangled_cur;
    int number = 0;
    for (; *p != '\0'; ++p)
    {
        if (IsDigit(*p))
        {
            number = number * 10 + (*p - '0');
        }
        else
        {
            break;
        }
    }
    if (p != state->mangled_cur)
    {  // Conversion succeeded.
        state->mangled_cur = p;
        if (number_out != NULL)
        {
            *number_out = number * sign;
        }
        return true;
    }
    return false;
}

// Floating-point literals are encoded using a fixed-length lowercase
// hexadecimal string.
static bool ParseFloatNumber(State *state)
{
    const char *p = state->mangled_cur;
    for (; *p != '\0'; ++p)
    {
        if (!IsDigit(*p) && !(*p >= 'a' && *p <= 'f'))
        {
            break;
        }
    }
    if (p != state->mangled_cur)
    {  // Conversion succeeded.
        state->mangled_cur = p;
        return true;
    }
    return false;
}

// The <seq-id> is a sequence number in base 36,
// using digits and upper case letters
static bool ParseSeqId(State *state)
{
    const char *p = state->mangled_cur;
    for (; *p != '\0'; ++p)
    {
        if (!IsDigit(*p) && !(*p >= 'A' && *p <= 'Z'))
        {
            break;
        }
    }
    if (p != state->mangled_cur)
    {  // Conversion succeeded.
        state->mangled_cur = p;
        return true;
    }
    return false;
}

// <identifier> ::= <unqualified source code identifier> (of given length)
static bool ParseIdentifier(State *state, int length)
{
    if (length == -1 ||
        !AtLeastNumCharsRemaining(state->mangled_cur, length))
    {
        return false;
    }
    if (IdentifierIsAnonymousNamespace(state, length))
    {
        MaybeAppend(state, "(anonymous namespace)");
    }
    else
    {
        MaybeAppendWithLength(state, state->mangled_cur, length);
    }
    state->mangled_cur += length;
    return true;
}

// <abi-tags> ::= <abi-tag> [<abi-tags>]
static bool ParseAbiTags(State *state)
{
    State copy = *state;
    DisableAppend(state);
    if (OneOrMore(ParseAbiTag, state))
    {
        RestoreAppend(state, copy.append);
        return true;
    }
    *state = copy;
    return false;
}

// <abi-tag> ::= B <source-name>
static bool ParseAbiTag(State *state)
{
    return ParseOneCharToken(state, 'B') && ParseSourceName(state);
}

// <operator-name> ::= nw, and other two letters cases
//                 ::= cv <type>  # (cast)
//                 ::= v  <digit> <source-name> # vendor extended operator
static bool ParseOperatorName(State *state)
{
    if (!AtLeastNumCharsRemaining(state->mangled_cur, 2))
    {
        return false;
    }
    // First check with "cv" (cast) case.
    State copy = *state;
    if (ParseTwoCharToken(state, "cv") &&
        MaybeAppend(state, "operator ") &&
        EnterNestedName(state) &&
        ParseType(state) &&
        LeaveNestedName(state, copy.nest_level))
    {
        return true;
    }
    *state = copy;

    // Then vendor extended operators.
    if (ParseOneCharToken(state, 'v') && ParseCharClass(state, "0123456789") &&
        ParseSourceName(state))
    {
        return true;
    }
    *state = copy;

    // Other operator names should start with a lower alphabet followed
    // by a lower/upper alphabet.
    if (!(IsLower(state->mangled_cur[0]) &&
          IsAlpha(state->mangled_cur[1])))
    {
        return false;
    }
    // We may want to perform a binary search if we really need speed.
    const AbbrevPair *p;
    for (p = kOperatorList; p->abbrev != NULL; ++p)
    {
        if (state->mangled_cur[0] == p->abbrev[0] &&
            state->mangled_cur[1] == p->abbrev[1])
        {
            MaybeAppend(state, "operator");
            if (IsLower(*p->real_name))
            {  // new, delete, etc.
                MaybeAppend(state, " ");
            }
            MaybeAppend(state, p->real_name);
            state->mangled_cur += 2;
            return true;
        }
    }
    return false;
}

// <special-name> ::= TV <type>
//                ::= TT <type>
//                ::= TI <type>
//                ::= TS <type>
//                ::= Tc <call-offset> <call-offset> <(base) encoding>
//                ::= GV <(object) name>
//                ::= T <call-offset> <(base) encoding>
// G++ extensions:
//                ::= TC <type> <(offset) number> _ <(base) type>
//                ::= TF <type>
//                ::= TJ <type>
//                ::= GR <name>
//                ::= GA <encoding>
//                ::= Th <call-offset> <(base) encoding>
//                ::= Tv <call-offset> <(base) encoding>
//
// Note: we don't care much about them since they don't appear in
// stack traces.  The are special data.
static bool ParseSpecialName(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'T') &&
        ParseCharClass(state, "VTIS") &&
        ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "Tc") && ParseCallOffset(state) &&
        ParseCallOffset(state) && ParseEncoding(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "GV") &&
        ParseName(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'T') && ParseCallOffset(state) &&
        ParseEncoding(state))
    {
        return true;
    }
    *state = copy;

    // G++ extensions
    if (ParseTwoCharToken(state, "TC") && ParseType(state) &&
        ParseNumber(state, NULL) && ParseOneCharToken(state, '_') &&
        DisableAppend(state) &&
        ParseType(state))
    {
        RestoreAppend(state, copy.append);
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'T') && ParseCharClass(state, "FJ") &&
        ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "GR") && ParseName(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "GA") && ParseEncoding(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'T') && ParseCharClass(state, "hv") &&
        ParseCallOffset(state) && ParseEncoding(state))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <call-offset> ::= h <nv-offset> _
//               ::= v <v-offset> _
static bool ParseCallOffset(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'h') &&
        ParseNVOffset(state) && ParseOneCharToken(state, '_'))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'v') &&
        ParseVOffset(state) && ParseOneCharToken(state, '_'))
    {
        return true;
    }
    *state = copy;

    return false;
}

// <nv-offset> ::= <(offset) number>
static bool ParseNVOffset(State *state)
{
    return ParseNumber(state, NULL);
}

// <v-offset>  ::= <(offset) number> _ <(virtual offset) number>
static bool ParseVOffset(State *state)
{
    State copy = *state;
    if (ParseNumber(state, NULL) && ParseOneCharToken(state, '_') &&
        ParseNumber(state, NULL))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <ctor-dtor-name> ::= C1 | C2 | C3
//                  ::= D0 | D1 | D2
static bool ParseCtorDtorName(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'C') &&
        ParseCharClass(state, "123"))
    {
        const char *const prev_name = state->prev_name;
        const int prev_name_length = state->prev_name_length;
        MaybeAppendWithLength(state, prev_name, prev_name_length);
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'D') &&
        ParseCharClass(state, "012"))
    {
        const char *const prev_name = state->prev_name;
        const int prev_name_length = state->prev_name_length;
        MaybeAppend(state, "~");
        MaybeAppendWithLength(state, prev_name, prev_name_length);
        return true;
    }
    *state = copy;
    return false;
}

// <type> ::= <CV-qualifiers> <type>
//        ::= P <type>   # pointer-to
//        ::= R <type>   # reference-to
//        ::= O <type>   # rvalue reference-to (C++0x)
//        ::= C <type>   # complex pair (C 2000)
//        ::= G <type>   # imaginary (C 2000)
//        ::= U <source-name> <type>  # vendor extended type qualifier
//        ::= <builtin-type>
//        ::= <function-type>
//        ::= <class-enum-type>
//        ::= <array-type>
//        ::= <pointer-to-member-type>
//        ::= <template-template-param> <template-args>
//        ::= <template-param>
//        ::= <substitution>
//        ::= Dp <type>          # pack expansion of (C++0x)
//        ::= Dt <expression> E  # decltype of an id-expression or class
//                               # member access (C++0x)
//        ::= DT <expression> E  # decltype of an expression (C++0x)
//
static bool ParseType(State *state)
{
    // We should check CV-qualifers, and PRGC things first.
    State copy = *state;
    if (ParseCVQualifiers(state) && ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseCharClass(state, "OPRCG") && ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "Dp") && ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'D') && ParseCharClass(state, "tT") &&
        ParseExpression(state) && ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'U') && ParseSourceName(state) &&
        ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseBuiltinType(state) ||
        ParseFunctionType(state) ||
        ParseClassEnumType(state) ||
        ParseArrayType(state) ||
        ParsePointerToMemberType(state) ||
        ParseSubstitution(state))
    {
        return true;
    }

    if (ParseTemplateTemplateParam(state) &&
        ParseTemplateArgs(state))
    {
        return true;
    }
    *state = copy;

    // Less greedy than <template-template-param> <template-args>.
    if (ParseTemplateParam(state))
    {
        return true;
    }

    return false;
}

// <CV-qualifiers> ::= [r] [V] [K]
// We don't allow empty <CV-qualifiers> to avoid infinite loop in
// ParseType().
static bool ParseCVQualifiers(State *state)
{
    int num_cv_qualifiers = 0;
    num_cv_qualifiers += ParseOneCharToken(state, 'r');
    num_cv_qualifiers += ParseOneCharToken(state, 'V');
    num_cv_qualifiers += ParseOneCharToken(state, 'K');
    return num_cv_qualifiers > 0;
}

// <builtin-type> ::= v, etc.
//                ::= u <source-name>
static bool ParseBuiltinType(State *state)
{
    const AbbrevPair *p;
    for (p = kBuiltinTypeList; p->abbrev != NULL; ++p)
    {
        if (state->mangled_cur[0] == p->abbrev[0])
        {
            MaybeAppend(state, p->real_name);
            ++state->mangled_cur;
            return true;
        }
    }

    State copy = *state;
    if (ParseOneCharToken(state, 'u') && ParseSourceName(state))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <function-type> ::= F [Y] <bare-function-type> E
static bool ParseFunctionType(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'F') &&
        Optional(ParseOneCharToken(state, 'Y')) &&
        ParseBareFunctionType(state) && ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <bare-function-type> ::= <(signature) type>+
static bool ParseBareFunctionType(State *state)
{
    State copy = *state;
    DisableAppend(state);
    if (OneOrMore(ParseType, state))
    {
        RestoreAppend(state, copy.append);
        MaybeAppend(state, "()");
        return true;
    }
    *state = copy;
    return false;
}

// <class-enum-type> ::= <name>
static bool ParseClassEnumType(State *state)
{
    return ParseName(state);
}

// <array-type> ::= A <(positive dimension) number> _ <(element) type>
//              ::= A [<(dimension) expression>] _ <(element) type>
static bool ParseArrayType(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'A') && ParseNumber(state, NULL) &&
        ParseOneCharToken(state, '_') && ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'A') && Optional(ParseExpression(state)) &&
        ParseOneCharToken(state, '_') && ParseType(state))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <pointer-to-member-type> ::= M <(class) type> <(member) type>
static bool ParsePointerToMemberType(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'M') && ParseType(state) &&
        ParseType(state))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <template-param> ::= T_
//                  ::= T <parameter-2 non-negative number> _
static bool ParseTemplateParam(State *state)
{
    if (ParseTwoCharToken(state, "T_"))
    {
        MaybeAppend(state, "?");  // We don't support template substitutions.
        return true;
    }

    State copy = *state;
    if (ParseOneCharToken(state, 'T') && ParseNumber(state, NULL) &&
        ParseOneCharToken(state, '_'))
    {
        MaybeAppend(state, "?");  // We don't support template substitutions.
        return true;
    }
    *state = copy;
    return false;
}


// <template-template-param> ::= <template-param>
//                           ::= <substitution>
static bool ParseTemplateTemplateParam(State *state)
{
    return (ParseTemplateParam(state) ||
            ParseSubstitution(state));
}

// <template-args> ::= I <template-arg>+ E
static bool ParseTemplateArgs(State *state)
{
    State copy = *state;
    DisableAppend(state);
    if (ParseOneCharToken(state, 'I') &&
        OneOrMore(ParseTemplateArg, state) &&
        ParseOneCharToken(state, 'E'))
    {
        RestoreAppend(state, copy.append);
        MaybeAppend(state, "<>");
        return true;
    }
    *state = copy;
    return false;
}

// <template-arg>  ::= <type>
//                 ::= <expr-primary>
//                 ::= I <template-arg>* E        # argument pack
//                 ::= J <template-arg>* E        # argument pack
//                 ::= X <expression> E
static bool ParseTemplateArg(State *state)
{
    State copy = *state;
    if ((ParseOneCharToken(state, 'I') || ParseOneCharToken(state, 'J')) &&
        ZeroOrMore(ParseTemplateArg, state) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;

    if (ParseType(state) ||
        ParseExprPrimary(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'X') && ParseExpression(state) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <expression> ::= <template-param>
//              ::= <expr-primary>
//              ::= <unary operator-name> <expression>
//              ::= <binary operator-name> <expression> <expression>
//              ::= <trinary operator-name> <expression> <expression>
//                  <expression>
//              ::= st <type>
//              ::= sr <type> <unqualified-name> <template-args>
//              ::= sr <type> <unqualified-name>
static bool ParseExpression(State *state)
{
    if (ParseTemplateParam(state) || ParseExprPrimary(state))
    {
        return true;
    }

    State copy = *state;
    if (ParseOperatorName(state) &&
        ParseExpression(state) &&
        ParseExpression(state) &&
        ParseExpression(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOperatorName(state) &&
        ParseExpression(state) &&
        ParseExpression(state))
    {
        return true;
    }
    *state = copy;

    if (ParseOperatorName(state) &&
        ParseExpression(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "st") && ParseType(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "sr") && ParseType(state) &&
        ParseUnqualifiedName(state) &&
        ParseTemplateArgs(state))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "sr") && ParseType(state) &&
        ParseUnqualifiedName(state))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <expr-primary> ::= L <type> <(value) number> E
//                ::= L <type> <(value) float> E
//                ::= L <mangled-name> E
//                // A bug in g++'s C++ ABI version 2 (-fabi-version=2).
//                ::= LZ <encoding> E
static bool ParseExprPrimary(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'L') && ParseType(state) &&
        ParseNumber(state, NULL) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'L') && ParseType(state) &&
        ParseFloatNumber(state) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'L') && ParseMangledName(state) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;

    if (ParseTwoCharToken(state, "LZ") && ParseEncoding(state) &&
        ParseOneCharToken(state, 'E'))
    {
        return true;
    }
    *state = copy;

    return false;
}

// <local-name> := Z <(function) encoding> E <(entity) name>
//                 [<discriminator>]
//              := Z <(function) encoding> E s [<discriminator>]
static bool ParseLocalName(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, 'Z') && ParseEncoding(state) &&
        ParseOneCharToken(state, 'E') && MaybeAppend(state, "::") &&
        ParseName(state) && Optional(ParseDiscriminator(state)))
    {
        return true;
    }
    *state = copy;

    if (ParseOneCharToken(state, 'Z') && ParseEncoding(state) &&
        ParseTwoCharToken(state, "Es") && Optional(ParseDiscriminator(state)))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <discriminator> := _ <(non-negative) number>
static bool ParseDiscriminator(State *state)
{
    State copy = *state;
    if (ParseOneCharToken(state, '_') && ParseNumber(state, NULL))
    {
        return true;
    }
    *state = copy;
    return false;
}

// <substitution> ::= S_
//                ::= S <seq-id> _
//                ::= St, etc.
static bool ParseSubstitution(State *state)
{
    if (ParseTwoCharToken(state, "S_"))
    {
        MaybeAppend(state, "?");  // We don't support substitutions.
        return true;
    }

    State copy = *state;
    if (ParseOneCharToken(state, 'S') && ParseSeqId(state) &&
        ParseOneCharToken(state, '_'))
    {
        MaybeAppend(state, "?");  // We don't support substitutions.
        return true;
    }
    *state = copy;

    // Expand abbreviations like "St" => "std".
    if (ParseOneCharToken(state, 'S'))
    {
        const AbbrevPair *p;
        for (p = kSubstitutionList; p->abbrev != NULL; ++p)
        {
            if (state->mangled_cur[0] == p->abbrev[1])
            {
                MaybeAppend(state, "std");
                if (p->real_name[0] != '\0')
                {
                    MaybeAppend(state, "::");
                    MaybeAppend(state, p->real_name);
                }
                ++state->mangled_cur;
                return true;
            }
        }
    }
    *state = copy;
    return false;
}

// Parse <mangled-name>, optionally followed by either a function-clone suffix
// or version suffix.  Returns true only if all of "mangled_cur" was consumed.
static bool ParseTopLevelMangledName(State *state)
{
    if (ParseMangledName(state))
    {
        if (state->mangled_cur[0] != '\0')
        {
            // Drop trailing function clone suffix, if any.
            if (IsFunctionCloneSuffix(state->mangled_cur))
            {
                return true;
            }
            // Append trailing version suffix if any.
            // ex. _Z3foo@@GLIBCXX_3.4
            if (state->mangled_cur[0] == '@')
            {
                MaybeAppend(state, state->mangled_cur);
                return true;
            }
            return false;  // Unconsumed suffix.
        }
        return true;
    }
    return false;
}

#endif

// The demangler entry point.
bool Demangle(const char *mangled, char *out, int out_size)
{
#if defined(OS_WINDOWS)
    // When built with incremental linking, the Windows debugger
  // library provides a more complicated `Symbol->Name` with the
  // Incremental Linking Table offset, which looks like
  // `@ILT+1105(?func@Foo@@SAXH@Z)`. However, the demangler expects
  // only the mangled symbol, `?func@Foo@@SAXH@Z`. Fortunately, the
  // mangled symbol is guaranteed not to have parentheses,
  // so we search for `(` and extract up to `)`.
  //
  // Since we may be in a signal handler here, we cannot use `std::string`.
  char buffer[1024];  // Big enough for a sane symbol.
  const char *lparen = strchr(mangled, '(');
  if (lparen) {
    // Extract the string `(?...)`
    const char *rparen = strchr(lparen, ')');
    size_t length = rparen - lparen - 1;
    strncpy(buffer, lparen + 1, length);
    buffer[length] = '\0';
    mangled = buffer;
  } // Else the symbol wasn't inside a set of parentheses
  // We use the ANSI version to ensure the string type is always `char *`.
  return UnDecorateSymbolName(mangled, out, out_size, UNDNAME_COMPLETE);
#else
    State state;
    InitState(&state, mangled, out, out_size);
    return ParseTopLevelMangledName(&state) && !state.overflowed;
#endif
}

class Backtrace
{
public:
    static void DumpStackTraceToString(string *stacktrace, int skip_count = 1)
    {
        DumpStackTrace(skip_count, DebugWriteToString, stacktrace);
    }

private:
    static void DebugWriteToString(const char *data, void *arg)
    {
        reinterpret_cast<string *>(arg)->append(data);
    }

    static void DumpPC(DebugWriter *writerfn, void *arg, void *pc, const char *const prefix)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "%s@ %*p\n", prefix, kPrintfPointerFieldWidth, pc);
        writerfn(buf, arg);
    }

    static void DumpStackTrace(int skip_count, DebugWriter *writerfn, void *arg)
    {
        // Print stack trace
        void *stack[32];
        int depth = GetStackTrace(stack, ARRAYSIZE(stack), skip_count + 1);
        for (int i = 0; i < depth; i++)
        {
            if (FLAGS_symbolize_stacktrace)
            {
                DumpPCAndSymbol(writerfn, arg, stack[i], std::to_string(i).c_str());
            }
            else
            {
                DumpPC(writerfn, arg, stack[i], "    ");
            }
        }
    }

    // If you change this function, also change GetStackFrames below.
    static int GetStackTrace(void **result, int max_depth, int skip_count)
    {
        static const int kStackLength = 64;
        void *stack[kStackLength];
        int size;

        size = backtrace(stack, kStackLength);
        skip_count++;  // we want to skip the current frame as well
        int result_count = size - skip_count;
        if (result_count < 0)
            result_count = 0;
        if (result_count > max_depth)
            result_count = max_depth;
        for (int i = 0; i < result_count; i++)
        {
            result[i] = stack[i + skip_count];
        }

        return result_count;
    }

    // Print a program counter and its symbol name.
    static void DumpPCAndSymbol(DebugWriter *writerfn, void *arg, void *pc,
                                const char *const prefix)
    {
        char tmp[1024];
        const char *symbol = "(unknown)";
        // Symbolizes the previous address of pc because pc may be in the
        // next function.  The overrun happens when the function ends with
        // a call to a function annotated noreturn (e.g. CHECK).
        if (Symbolize(reinterpret_cast<char *>(pc) - 1, tmp, sizeof(tmp)))
        {
            symbol = tmp;
        }
        char buf[1024];
        snprintf(buf, sizeof(buf), "#%s %*p | %s\n", prefix, kPrintfPointerFieldWidth, pc, symbol);
        writerfn(buf, arg);
    }

    static bool Symbolize(void *pc, char *out, int out_size)
    {
        SAFE_ASSERT(out_size >= 0);
        return SymbolizeAndDemangle(pc, out, out_size);
    }

    // The implementation of our symbolization routine.  If it
    // successfully finds the symbol containing "pc" and obtains the
    // symbol name, returns true and write the symbol name to "out".
    // Otherwise, returns false. If Callback function is installed via
    // InstallSymbolizeCallback(), the function is also called in this function,
    // and "out" is used as its output.
    // To keep stack consumption low, we would like this function to not
    // get inlined.
    static __attribute__ ((noinline)) bool SymbolizeAndDemangle(void *pc, char *out, int out_size)
    {
        uint64_t pc0 = reinterpret_cast<uintptr_t>(pc);
        uint64_t start_address = 0;
        uint64_t base_address = 0;
        int object_fd = -1;

        if (out_size < 1)
        {
            return false;
        }
        out[0] = '\0';
        SafeAppendString("(", out, out_size);

        if (g_symbolize_open_object_file_callback)
        {
            object_fd = g_symbolize_open_object_file_callback(pc0, start_address,
                                                              base_address, out + 1,
                                                              out_size - 1);
        }
        else
        {
            object_fd = OpenObjectFileContainingPcAndGetStartAddress(pc0, start_address,
                                                                     base_address,
                                                                     out + 1,
                                                                     out_size - 1);
        }

        FileDescriptor wrapped_object_fd(object_fd);

        // Check whether a file name was returned.
        if (object_fd < 0)
        {
            if (out[1])
            {
                // The object file containing PC was determined successfully however the
                // object file was not opened successfully.  This is still considered
                // success because the object file name and offset are known and tools
                // like asan_symbolize.py can be used for the symbolization.
                out[out_size - 1] = '\0';  // Making sure |out| is always null-terminated.
                SafeAppendString("+0x", out, out_size);
                SafeAppendHexNumber(pc0 - base_address, out, out_size);
                SafeAppendString(")", out, out_size);
                return true;
            }
            // Failed to determine the object file containing PC.  Bail out.
            return false;
        }
        int elf_type = FileGetElfType(wrapped_object_fd.get());
        if (elf_type == -1)
        {
            return false;
        }
        if (g_symbolize_callback)
        {
            // Run the call back if it's installed.
            // Note: relocation (and much of the rest of this code) will be
            // wrong for prelinked shared libraries and PIE executables.
            uint64_t relocation = (elf_type == ET_DYN) ? start_address : 0;
            int num_bytes_written = g_symbolize_callback(wrapped_object_fd.get(),
                                                         pc, out, out_size,
                                                         relocation);
            if (num_bytes_written > 0)
            {
                out += num_bytes_written;
                out_size -= num_bytes_written;
            }
        }
        if (!GetSymbolFromObjectFile(wrapped_object_fd.get(), pc0,
                                     out, out_size, base_address))
        {
            if (out[1] && !g_symbolize_callback)
            {
                // The object file containing PC was opened successfully however the
                // symbol was not found. The object may have been stripped. This is still
                // considered success because the object file name and offset are known
                // and tools like asan_symbolize.py can be used for the symbolization.
                out[out_size - 1] = '\0';  // Making sure |out| is always null-terminated.
                SafeAppendString("+0x", out, out_size);
                SafeAppendHexNumber(pc0 - base_address, out, out_size);
                SafeAppendString(")", out, out_size);
                return true;
            }
            return false;
        }

        // Symbolization succeeded.  Now we try to demangle the symbol.
        DemangleInplace(out, out_size);
        return true;
    }

    // Safely appends string |source| to string |dest|.  Never writes past the
    // buffer size |dest_size| and guarantees that |dest| is null-terminated.
    static void SafeAppendString(const char *source, char *dest, int dest_size)
    {
        int dest_string_length = strlen(dest);
        SAFE_ASSERT(dest_string_length < dest_size);
        dest += dest_string_length;
        dest_size -= dest_string_length;
        strncpy(dest, source, dest_size);
        // Making sure |dest| is always null-terminated.
        dest[dest_size - 1] = '\0';
    }

    // Searches for the object file (from /proc/self/maps) that contains
    // the specified pc.  If found, sets |start_address| to the start address
    // of where this object file is mapped in memory, sets the module base
    // address into |base_address|, copies the object file name into
    // |out_file_name|, and attempts to open the object file.  If the object
    // file is opened successfully, returns the file descriptor.  Otherwise,
    // returns -1.  |out_file_name_size| is the size of the file name buffer
    // (including the null-terminator).
    static __attribute__ ((noinline)) int
    OpenObjectFileContainingPcAndGetStartAddress(uint64_t pc,
                                                 uint64_t &start_address,
                                                 uint64_t &base_address,
                                                 char *out_file_name,
                                                 int out_file_name_size)
    {
        int object_fd;

        int maps_fd;
        NO_INTR(maps_fd = open("/proc/self/maps", O_RDONLY));
        FileDescriptor wrapped_maps_fd(maps_fd);
        if (wrapped_maps_fd.get() < 0)
        {
            return -1;
        }

        int mem_fd;
        NO_INTR(mem_fd = open("/proc/self/mem", O_RDONLY));
        FileDescriptor wrapped_mem_fd(mem_fd);
        if (wrapped_mem_fd.get() < 0)
        {
            return -1;
        }

        // Iterate over maps and look for the map containing the pc.  Then
        // look into the symbol tables inside.
        char buf[1024];  // Big enough for line of sane /proc/self/maps
        int num_maps = 0;
        LineReader reader(wrapped_maps_fd.get(), buf, sizeof(buf), 0);
        while (true)
        {
            num_maps++;
            const char *cursor;
            const char *eol;
            if (!reader.ReadLine(&cursor, &eol))
            {  // EOF or malformed line.
                return -1;
            }

            // Start parsing line in /proc/self/maps.  Here is an example:
            //
            // 08048000-0804c000 r-xp 00000000 08:01 2142121    /bin/cat
            //
            // We want start address (08048000), end address (0804c000), flags
            // (r-xp) and file name (/bin/cat).

            // Read start address.
            cursor = GetHex(cursor, eol, &start_address);
            if (cursor == eol || *cursor != '-')
            {
                return -1;  // Malformed line.
            }
            ++cursor;  // Skip '-'.

            // Read end address.
            uint64_t end_address;
            cursor = GetHex(cursor, eol, &end_address);
            if (cursor == eol || *cursor != ' ')
            {
                return -1;  // Malformed line.
            }
            ++cursor;  // Skip ' '.

            // Read flags.  Skip flags until we encounter a space or eol.
            const char *const flags_start = cursor;
            while (cursor < eol && *cursor != ' ')
            {
                ++cursor;
            }
            // We expect at least four letters for flags (ex. "r-xp").
            if (cursor == eol || cursor < flags_start + 4)
            {
                return -1;  // Malformed line.
            }

            // Determine the base address by reading ELF headers in process memory.
            ElfW(Ehdr) ehdr;
            // Skip non-readable maps.
            if (flags_start[0] == 'r' &&
                ReadFromOffsetExact(mem_fd, &ehdr, sizeof(ElfW(Ehdr)), start_address) &&
                memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0)
            {
                switch (ehdr.e_type)
                {
                    case ET_EXEC:
                        base_address = 0;
                        break;
                    case ET_DYN:
                        // Find the segment containing file offset 0. This will correspond
                        // to the ELF header that we just read. Normally this will have
                        // virtual address 0, but this is not guaranteed. We must subtract
                        // the virtual address from the address where the ELF header was
                        // mapped to get the base address.
                        //
                        // If we fail to find a segment for file offset 0, use the address
                        // of the ELF header as the base address.
                        base_address = start_address;
                        for (unsigned i = 0; i != ehdr.e_phnum; ++i)
                        {
                            ElfW(Phdr) phdr;
                            if (ReadFromOffsetExact(
                                    mem_fd, &phdr, sizeof(phdr),
                                    start_address + ehdr.e_phoff + i * sizeof(phdr)) &&
                                phdr.p_type == PT_LOAD && phdr.p_offset == 0)
                            {
                                base_address = start_address - phdr.p_vaddr;
                                break;
                            }
                        }
                        break;
                    default:
                        // ET_REL or ET_CORE. These aren't directly executable, so they don't
                        // affect the base address.
                        break;
                }
            }

            // Check start and end addresses.
            if (!(start_address <= pc && pc < end_address))
            {
                continue;  // We skip this map.  PC isn't in this map.
            }

            // Check flags.  We are only interested in "r*x" maps.
            if (flags_start[0] != 'r' || flags_start[2] != 'x')
            {
                continue;  // We skip this map.
            }
            ++cursor;  // Skip ' '.

            // Read file offset.
            uint64_t file_offset;
            cursor = GetHex(cursor, eol, &file_offset);
            if (cursor == eol || *cursor != ' ')
            {
                return -1;  // Malformed line.
            }
            ++cursor;  // Skip ' '.

            // Skip to file name.  "cursor" now points to dev.  We need to
            // skip at least two spaces for dev and inode.
            int num_spaces = 0;
            while (cursor < eol)
            {
                if (*cursor == ' ')
                {
                    ++num_spaces;
                }
                else if (num_spaces >= 2)
                {
                    // The first non-space character after skipping two spaces
                    // is the beginning of the file name.
                    break;
                }
                ++cursor;
            }
            if (cursor == eol)
            {
                return -1;  // Malformed line.
            }

            // Finally, "cursor" now points to file name of our interest.
            NO_INTR(object_fd = open(cursor, O_RDONLY));
            if (object_fd < 0)
            {
                // Failed to open object file.  Copy the object file name to
                // |out_file_name|.
                strncpy(out_file_name, cursor, out_file_name_size);
                // Making sure |out_file_name| is always null-terminated.
                out_file_name[out_file_name_size - 1] = '\0';
                return -1;
            }
            return object_fd;
        }
    }

    // This function wraps the Demangle function to provide an interface
// where the input symbol is demangled in-place.
// To keep stack consumption low, we would like this function to not
// get inlined.
    static ATTRIBUTE_NOINLINE void DemangleInplace(char *out, int out_size)
    {
        char demangled[256];  // Big enough for sane demangled symbols.
        if (Demangle(out, demangled, sizeof(demangled)))
        {
            // Demangling succeeded. Copy to out if the space allows.
            size_t len = strlen(demangled);
            if (len + 1 <= (size_t) out_size)
            {  // +1 for '\0'.
                SAFE_ASSERT(len < sizeof(demangled));
                memmove(out, demangled, len + 1);
            }
        }
    }

    // Converts a 64-bit value into a hex string, and safely appends it to |dest|.
// Never writes past the buffer size |dest_size| and guarantees that |dest| is
// null-terminated.
    static void SafeAppendHexNumber(uint64_t value, char *dest, int dest_size)
    {
        // 64-bit numbers in hex can have up to 16 digits.
        char buf[17] = {'\0'};
        SafeAppendString(itoa_r(value, buf, sizeof(buf), 16, 0), dest, dest_size);
    }

    // POSIX doesn't define any async-signal safe function for converting
// an integer to ASCII. We'll have to define our own version.
// itoa_r() converts a (signed) integer to ASCII. It returns "buf", if the
// conversion was successful or NULL otherwise. It never writes more than "sz"
// bytes. Output will be truncated as needed, and a NUL character is always
// appended.
// NOTE: code from sandbox/linux/seccomp-bpf/demo.cc.
    static char *itoa_r(intptr_t i, char *buf, size_t sz, int base, size_t padding)
    {
        // Make sure we can write at least one NUL byte.
        size_t n = 1;
        if (n > sz)
            return NULL;

        if (base < 2 || base > 16)
        {
            buf[0] = '\000';
            return NULL;
        }

        char *start = buf;

        uintptr_t j = i;

        // Handle negative numbers (only for base 10).
        if (i < 0 && base == 10)
        {
            // This does "j = -i" while avoiding integer overflow.
            j = static_cast<uintptr_t>(-(i + 1)) + 1;

            // Make sure we can write the '-' character.
            if (++n > sz)
            {
                buf[0] = '\000';
                return NULL;
            }
            *start++ = '-';
        }

        // Loop until we have converted the entire number. Output at least one
        // character (i.e. '0').
        char *ptr = start;
        do
        {
            // Make sure there is still enough space left in our output buffer.
            if (++n > sz)
            {
                buf[0] = '\000';
                return NULL;
            }

            // Output the next digit.
            *ptr++ = "0123456789abcdef"[j % base];
            j /= base;

            if (padding > 0)
                padding--;
        } while (j > 0 || padding > 0);

        // Terminate the output with a NUL character.
        *ptr = '\000';

        // Conversion to ASCII actually resulted in the digits being in reverse
        // order. We can't easily generate them in forward order, as we can't tell
        // the number of characters needed until we are done converting.
        // So, now, we reverse the string (except for the possible "-" sign).
        while (--ptr > start)
        {
            char ch = *ptr;
            *ptr = *start;
            *start++ = ch;
        }
        return buf;
    }

    // Returns elf_header.e_type if the file pointed by fd is an ELF binary.
    static int FileGetElfType(const int fd)
    {
        ElfW(Ehdr) elf_header;
        if (!ReadFromOffsetExact(fd, &elf_header, sizeof(elf_header), 0))
        {
            return -1;
        }
        if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0)
        {
            return -1;
        }
        return elf_header.e_type;
    }

    // Try reading exactly "count" bytes from "offset" bytes in a file
    // pointed by "fd" into the buffer starting at "buf" while handling
    // short reads and EINTR.  On success, return true. Otherwise, return
    // false.
    static bool ReadFromOffsetExact(const int fd, void *buf,
                                    const size_t count, const off_t offset)
    {
        size_t len = ReadFromOffset(fd, buf, count, offset);
        return len == count;
    }

    // Get the symbol name of "pc" from the file pointed by "fd".  Process
    // both regular and dynamic symbol tables if necessary.  On success,
    // write the symbol name to "out" and return true.  Otherwise, return
    // false.
    static bool GetSymbolFromObjectFile(const int fd,
                                        uint64_t pc,
                                        char *out,
                                        int out_size,
                                        uint64_t base_address)
    {
        // Read the ELF header.
        ElfW(Ehdr) elf_header;
        if (!ReadFromOffsetExact(fd, &elf_header, sizeof(elf_header), 0))
        {
            return false;
        }

        ElfW(Shdr) symtab, strtab;

        // Consult a regular symbol table first.
        if (GetSectionHeaderByType(fd, elf_header.e_shnum, elf_header.e_shoff,
                                   SHT_SYMTAB, &symtab))
        {
            if (!ReadFromOffsetExact(fd, &strtab, sizeof(strtab), elf_header.e_shoff +
                                                                  symtab.sh_link * sizeof(symtab)))
            {
                return false;
            }
            if (FindSymbol(pc, fd, out, out_size, base_address, &strtab, &symtab))
            {
                return true;  // Found the symbol in a regular symbol table.
            }
        }

        // If the symbol is not found, then consult a dynamic symbol table.
        if (GetSectionHeaderByType(fd, elf_header.e_shnum, elf_header.e_shoff,
                                   SHT_DYNSYM, &symtab))
        {
            if (!ReadFromOffsetExact(fd, &strtab, sizeof(strtab), elf_header.e_shoff +
                                                                  symtab.sh_link * sizeof(symtab)))
            {
                return false;
            }
            if (FindSymbol(pc, fd, out, out_size, base_address, &strtab, &symtab))
            {
                return true;  // Found the symbol in a dynamic symbol table.
            }
        }

        return false;
    }

    // Read the section headers in the given ELF binary, and if a section
// of the specified type is found, set the output to this section header
// and return true.  Otherwise, return false.
// To keep stack consumption low, we would like this function to not get
// inlined.
    static ATTRIBUTE_NOINLINE bool
    GetSectionHeaderByType(const int fd, ElfW(Half) sh_num, const off_t sh_offset,
                           ElfW(Word) type, ElfW(Shdr) *out)
    {
        // Read at most 16 section headers at a time to save read calls.
        ElfW(Shdr) buf[16];
        for (int i = 0; i < sh_num;)
        {
            const size_t num_bytes_left = (sh_num - i) * sizeof(buf[0]);
            const auto num_bytes_to_read =
                    (sizeof(buf) > num_bytes_left) ? num_bytes_left : sizeof(buf);
            const ssize_t len = ReadFromOffset(fd, buf, num_bytes_to_read,
                                               sh_offset + i * sizeof(buf[0]));
            if (len == -1)
            {
                return false;
            }
            SAFE_ASSERT(len % sizeof(buf[0]) == 0);
            const size_t num_headers_in_buf = len / sizeof(buf[0]);
            SAFE_ASSERT(num_headers_in_buf <= sizeof(buf) / sizeof(buf[0]));
            for (size_t j = 0; j < num_headers_in_buf; ++j)
            {
                if (buf[j].sh_type == type)
                {
                    *out = buf[j];
                    return true;
                }
            }
            i += num_headers_in_buf;
        }
        return false;
    }

    // Read a symbol table and look for the symbol containing the
// pc. Iterate over symbols in a symbol table and look for the symbol
// containing "pc".  On success, return true and write the symbol name
// to out.  Otherwise, return false.
// To keep stack consumption low, we would like this function to not get
// inlined.
    static ATTRIBUTE_NOINLINE bool
    FindSymbol(uint64_t pc, const int fd, char *out, int out_size,
               uint64_t symbol_offset, const ElfW(Shdr) *strtab,
               const ElfW(Shdr) *symtab)
    {
        if (symtab == NULL)
        {
            return false;
        }
        const int num_symbols = symtab->sh_size / symtab->sh_entsize;
        for (int i = 0; i < num_symbols;)
        {
            off_t offset = symtab->sh_offset + i * symtab->sh_entsize;

            // If we are reading Elf64_Sym's, we want to limit this array to
            // 32 elements (to keep stack consumption low), otherwise we can
            // have a 64 element Elf32_Sym array.
#if __WORDSIZE == 64
#define NUM_SYMBOLS 32
#else
#define NUM_SYMBOLS 64
#endif

            // Read at most NUM_SYMBOLS symbols at once to save read() calls.
            ElfW(Sym) buf[NUM_SYMBOLS];
            int num_symbols_to_read = std::min(NUM_SYMBOLS, num_symbols - i);
            const ssize_t len =
                    ReadFromOffset(fd, &buf, sizeof(buf[0]) * num_symbols_to_read, offset);
            SAFE_ASSERT(len % sizeof(buf[0]) == 0);
            const ssize_t num_symbols_in_buf = len / sizeof(buf[0]);
            SAFE_ASSERT(num_symbols_in_buf <= num_symbols_to_read);
            for (int j = 0; j < num_symbols_in_buf; ++j)
            {
                const ElfW(Sym) &symbol = buf[j];
                uint64_t start_address = symbol.st_value;
                start_address += symbol_offset;
                uint64_t end_address = start_address + symbol.st_size;
                if (symbol.st_value != 0 &&  // Skip null value symbols.
                    symbol.st_shndx != 0 &&  // Skip undefined symbols.
                    start_address <= pc && pc < end_address)
                {
                    ssize_t len1 = ReadFromOffset(fd, out, out_size,
                                                  strtab->sh_offset + symbol.st_name);
                    if (len1 <= 0 || memchr(out, '\0', out_size) == NULL)
                    {
                        memset(out, 0, out_size);
                        return false;
                    }
                    return true;  // Obtained the symbol name.
                }
            }
            i += num_symbols_in_buf;
        }
        return false;
    }
};
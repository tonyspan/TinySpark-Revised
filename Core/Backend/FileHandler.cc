#include <boost/tokenizer.hpp>

#include "../pch.h"
#include "FileHandler.h"

namespace TinySpark
{
    FileHandler::FileHandler() {}

    FileHandler::~FileHandler() {}

    void FileHandler::OpenFile(const std::string &filename, bool driver)
    {
        m_FileName = filename;
        m_FileSizeBytes = GetSizeForRequestedFile(filename);
        m_FilePath = GetSizeForRequestedFile(filename);
        if (driver)
            CalcChunks();
        m_Stream.open(GetPathForRequestedFile(filename), std::ios::in | std::ios::binary);
        if (!m_Stream)
            throw std::runtime_error(filename + ": " + std::strerror(errno));
    }

    void FileHandler::CloseFile() { m_Stream.close(); }

    bool FileHandler::IsOpenFile() { return m_Stream.is_open(); }

    std::vector<char> FileHandler::ParseFile(const std::string &filepath, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset)
    {
        if (!m_Stream)
            throw std::runtime_error("TinySpark: " + filepath + ": " + std::strerror(errno));

        m_Stream.clear();
        m_Stream.seekg(chunkOffset, std::ios::beg);

        if (chunkSize == 0) // avoid undefined behavior
            return {};

        std::vector<char> buffer(chunkSize);

        if (!m_Stream.read(&buffer[0], buffer.size()))
            throw std::runtime_error("TinySpark: " + filepath + ": " + std::strerror(errno));

        return buffer;
    }

    // Unused
    std::vector<char> FileHandler::ParseFileAsync(const std::string filepath, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset)
    {
        if (!m_Stream)
            throw std::runtime_error(filepath + ": " + std::strerror(errno));

        m_Stream.clear();
        m_Stream.seekg(chunkOffset, std::ios::beg);

        if (chunkSize == 0) // avoid undefined behavior
            return {};

        std::vector<char> buffer(chunkSize);

        if (!m_Stream.read(&buffer[0], buffer.size()))
            throw std::runtime_error(filepath + ": " + std::strerror(errno));

        return buffer;
    }

    std::string FileHandler::GetFileName() const { return m_FileName; }

    void FileHandler::SetNewChunkSize(std::uintmax_t newChunkSize) { m_ChunkSizeBytes = newChunkSize; }

    std::uintmax_t FileHandler::GetChunks() const { return m_Chunks; }

    std::uintmax_t FileHandler::GetChunkSizeBytes() const { return m_ChunkSizeBytes; }

    std::uintmax_t FileHandler::GetLastChunkSizeBytes() const { return m_LastChunkSizeBytes; }

    std::uintmax_t FileHandler::GetSizeOpenedFile() const { return m_FileSizeBytes; }

    std::string FileHandler::GetPathForOpenedFile() const { return m_FilePath; }

    std::uintmax_t FileHandler::GetSizeForRequestedFile(const std::string &filename) const { return std::filesystem::file_size(GetPathForRequestedFile(filename)); }

    std::string FileHandler::GetPathForRequestedFile(const std::string &filename) const
    {
        auto currPath = std::filesystem::current_path();

        auto pathIter = std::filesystem::recursive_directory_iterator(currPath.root_path(), std::filesystem::directory_options::skip_permission_denied);

        auto found = std::find_if(pathIter, end(pathIter), [&filename](const auto &dir_entry) {
            return dir_entry.path().filename() == filename;
        });

        if (found != end(pathIter))
        {
            // it's const
            auto path = found->path();
            return path.make_preferred();
        }
        throw std::runtime_error("TinySpark: " + filename + ": " + std::strerror(errno));
    }

    std::vector<std::string> FileHandler::TokenizeFile(const std::string &inputStr, const std::string &delims, bool desensitize)
    {
        boost::char_separator<char> sep(delims.c_str(), "", boost::keep_empty_tokens);
        boost::tokenizer<boost::char_separator<char>> tokens(inputStr, sep);

        std::vector<std::string> output{ std::begin(tokens), std::end(tokens) };
        output.erase(std::end(output));

        if (desensitize)
        {
            RemoveOneLetterWord(output);
            ToLowerCase(output);
            RemoveDigits(output);
        }

        return output;
    }

    void FileHandler::CalcChunks()
    {
        m_Chunks = m_FileSizeBytes / m_ChunkSizeBytes;
        m_LastChunkSizeBytes = m_FileSizeBytes % m_ChunkSizeBytes;

        if (m_LastChunkSizeBytes != 0)
            ++m_Chunks;
        else
            m_LastChunkSizeBytes = m_ChunkSizeBytes;
    }

    void FileHandler::RemoveOneLetterWord(std::vector<std::string> &vs)
    {
        vs.erase(std::remove_if(vs.begin(), vs.end(), [](const std::string &el) { return ((el.size() == 1) && (el != "I" || el != "a")); }), vs.end());
    }

    void FileHandler::RemoveDigits(std::vector<std::string> &vs)
    {

        vs.erase(std::remove_if(vs.begin(), vs.end(), [](const std::string &el)
                                                    {
                                                        return !el.empty() && std::find_if(el.begin(), el.end(), [](unsigned char c) { return !std::isdigit(c); }) == el.end();
                                                        ;
                                                    }),
                                                    vs.end());
    }

    void FileHandler::ToLowerCase(std::vector<std::string> &vs)
    {
        std::transform(vs.begin(), vs.end(), vs.begin(), [](const std::string &el)
                                                        {
                                                            std::string out;
                                                            out.reserve(el.size());
                                                            std::transform(el.begin(), el.end(), std::back_inserter(out), ::tolower);
                                                            return out;
                                                        });
    }
} // namespace TinySpark
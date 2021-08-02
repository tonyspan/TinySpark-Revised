#pragma once

namespace TinySpark
{
    class FileHandler
    {
    public:
        FileHandler();

        ~FileHandler();

        // bool parameter only true for driver
        void OpenFile(const std::string &filename, bool driver = true);

        void CloseFile();

        bool IsOpenFile();

        std::vector<char> ParseFile(const std::string &filepath, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset);

        // Unused
        std::vector<char> ParseFileAsync(const std::string filepath, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset);

        // Default is 128MB
        void SetNewChunkSize(std::uintmax_t newSize);

        std::string GetFileName() const;

        std::uintmax_t GetChunks() const;

        std::uintmax_t GetChunkSizeBytes() const;

        std::uintmax_t GetLastChunkSizeBytes() const;

        std::uintmax_t GetSizeOpenedFile() const;

        std::string GetPathForOpenedFile() const;

        // Returns size in Bytes of the file that is indicated by filename
        // Doesn't open file for read
        std::uintmax_t GetSizeForRequestedFile(const std::string &filename) const;

        // Returns path of the file that is indicated by filename
        // Doesn't open file for read
        std::string GetPathForRequestedFile(const std::string &filename) const;

        std::vector<std::string> TokenizeFile(const std::string &inputStr, const std::string &delims = " ", bool desensitize = false);

    private:
        void CalcChunks();

        void RemoveOneLetterWord(std::vector<std::string> &vs);

        void RemoveDigits(std::vector<std::string> &vs);

        void ToLowerCase(std::vector<std::string> &vs);

    private:
        std::ifstream m_Stream;
        std::uintmax_t m_FileSizeBytes;

        std::uintmax_t m_ChunkSizeBytes = 128 * 1024 * 1024; // 128MB
        std::uintmax_t m_LastChunkSizeBytes = 0;
        std::uintmax_t m_Chunks = 0;
        std::string m_FileName;
        std::string m_FilePath;
    };
} // namespace TinySpark
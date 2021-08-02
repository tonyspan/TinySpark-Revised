#pragma once

struct DataToSent
{
    std::string MapForm;
    std::string MapRet;

    std::string ReduceForm;
    std::string ReduceRet;

    std::string FilterForm;
    std::string FilterRet;
    std::string HasFilter;

    int ArgType;
    float initial = 0;
};

class DiscoverSericeImplDriver final : public Utils::UtilsService::Service
{
    grpc::Status registerWorker(grpc::ServerContext *context, const Utils::RegisterRequest *request, Utils::RegisterResponse *response) override;

    // Unused, written for fun
    // TODO: Probably causes duplicate instances of workers in g_WorkerList
    grpc::Status deRegisterWorker(grpc::ServerContext *context, const Utils::RegisterRequest *request, Utils::EmptyResponse *response) override;
};

class MFRServiceImplDriver final : public MFR::MFRService::Service
{
    grpc::Status MFRListCD(grpc::ServerContext *context, const MFR::MFRListRequest *request, MFR::MFRResponse *response) override;
    
    grpc::Status MFRFileCD(grpc::ServerContext *context, const MFR::MFRFileRequest *request, MFR::MFRResponse *response) override;
};

class Driver
{
public:
    Driver();

    ~Driver();

    void SetChannel(std::shared_ptr<grpc::Channel> channel, const std::string &service);

    void pingWorker(const std::string &addr);

    float MFRListDW(const std::vector<float> &values, const DataToSent &data);

    float MFRListDW(const std::vector<std::string> &values, const DataToSent &data);

    std::unordered_map<std::string, uint32_t> MFRFileMapDW(const std::string &filename, const std::string &mapAction, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset);

    uint32_t MFRFileNonMapDW(const std::string &filename, const std::string &mapAction, const std::uintmax_t chunkSize, const std::uintmax_t chunkOffset);

private:
    std::unique_ptr<Utils::UtilsService::Stub> m_UtilsStub;
    std::unique_ptr<MFR::MFRService::Stub> m_MFRStub;
};
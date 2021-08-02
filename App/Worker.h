#pragma once

class MFRServiceImplWorker : public MFR::MFRService::Service
{
    grpc::Status MFRListDW(grpc::ServerContext *context, const MFR::MFRListRequest *request, MFR::MFRResponse *response) override;

    grpc::Status MFRFileDW(grpc::ServerContext *context, const MFR::MFRFileRequest *request, MFR::MFRResponse *response) override;

};

class Worker
{
public:
    Worker(std::shared_ptr<grpc::Channel>);

    ~Worker();

    void registerWorker(const std::string &addr);

    void deRegisterWorker(const std::string &addr);

    uint32_t GetWorkerID();

private:
    std::unique_ptr<Utils::UtilsService::Stub> m_Stub;
    uint32_t m_WorkerID = 0;
};
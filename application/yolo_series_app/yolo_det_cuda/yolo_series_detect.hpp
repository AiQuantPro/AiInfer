#ifndef _YOLOV_SERIES_DETECT_CUDA_HPP_
#define _YOLOV_SERIES_DETECT_CUDA_HPP_
#include <memory>
#include "backend/tensorrt/trt_infer.hpp"
#include "common/model_info.hpp"
#include "common/utils.hpp"
#include "common/cv_cpp_utils.hpp"
#include "common/memory.hpp"
#include "pre_process/pre_process.cuh"
#include "post_process/post_process.cuh"

namespace tensorrt_infer
{
    namespace yolo_series_cuda
    {
        using namespace ai::modelInfo;
        using namespace ai::utils;
        using namespace ai::cvUtil;
        using namespace ai::memory;
        using namespace ai::preprocess;
        using namespace ai::postprocess;

        enum class Type : int
        {
            V5 = 0,
            X = 1,
            V6 = 2,
            V7 = 3
        };

        Type name_type(const std::string &model_name)
        {
            if (!strcmp(model_name.c_str(), "yolov5"))
                return Type::V5;
            else if (!strcmp(model_name.c_str(), "yolox"))
                return Type::X;
            else if (!strcmp(model_name.c_str(), "yolov6"))
                return Type::V6;
            else if (!strcmp(model_name.c_str(), "yolov7"))
                return Type::V7;
            else
                exit(-1);
        }

        class YOLOSeriesDetect
        {
        public:
            YOLOSeriesDetect() = default;
            ~YOLOSeriesDetect();
            void initParameters(const std::string &model_name, const std::string &engine_file, float score_thr = 0.5f,
                                float nms_thr = 0.45f); // 初始化参数
            void adjust_memory(int batch_size);         // 由于batch size是动态的，所以需要对gpu/cpu内存进行动态的申请

            // forward
            BoxArray forward(const Image &image);
            BatchBoxArray forwards(const std::vector<Image> &images);

            // 模型前后处理
            void preprocess_gpu(int ibatch, const Image &image,
                                shared_ptr<Memory<unsigned char>> preprocess_buffer, AffineMatrix &affine,
                                cudaStream_t stream_);
            void postprocess_gpu(int ibatch, cudaStream_t stream_);
            BatchBoxArray parser_box(int num_image);

        private:
            // model name
            Type model_type;

            std::shared_ptr<ai::backend::Infer> model_;
            std::shared_ptr<ModelInfo> model_info = nullptr;

            // 仿射矩阵的声明
            std::vector<AffineMatrix> affine_matrixs;
            const uint8_t const_value = 114; // 图片resize补边时的值

            // 使用自定义的Memory类用来申请gpu/cpu内存
            std::vector<std::shared_ptr<Memory<unsigned char>>> preprocess_buffers_;
            Memory<float> input_buffer_, bbox_predict_, output_boxarray_;

            // 使用cuda流进行操作
            cudaStream_t cu_stream;

            // time
            Timer timer;
        };
    }
}

#endif // _YOLOV_SERIES_DETECT_CUDA_HPP_
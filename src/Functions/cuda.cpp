#include "config_functions.h"

// #if ENABLE_CUDA

#include <Columns/ColumnString.h>
#include <DataTypes/DataTypeString.h>
#include <Functions/FunctionFactory.h>
#include <Functions/FunctionHelpers.h>
#include <Functions/IFunction.h>
#include <Interpreters/Context.h>

#if USE_CUDA
// #include <cuda_func.h>
std::string counter_cuda(float a);
#else
std::string counter_cuda(float /*a*/)
{
    return "Not implemented :(";
}
#endif


namespace DB
{
namespace ErrorCodes
{
    extern const int ILLEGAL_COLUMN;
    extern const int ILLEGAL_TYPE_OF_ARGUMENT;
    extern const int SUPPORT_IS_DISABLED;
}

namespace
{

class FunctionCuda : public IFunction
{
public:
    static constexpr auto name = "cuda";

    static FunctionPtr create(ContextPtr /*context*/)
    {
        // if (!context->getSettingsRef().allow_experimental_nlp_functions)
        //     throw Exception(ErrorCodes::SUPPORT_IS_DISABLED, "Natural language processing function '{}' is experimental. Set `allow_experimental_nlp_functions` setting to enable it", name);

        return std::make_shared<FunctionCuda>();
    }

    String getName() const override { return name; }

    size_t getNumberOfArguments() const override { return 0; }

    bool isSuitableForShortCircuitArgumentsExecution(const DataTypesWithConstInfo & /*arguments*/) const override { return true; }

    DataTypePtr getReturnTypeImpl(const DataTypes & /*arguments*/) const override
    {
        // if (!isString(arguments[0]))
        //     throw Exception(
        //         "Illegal type " + arguments[0]->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);
        // if (!isString(arguments[1]))
        //     throw Exception(
        //         "Illegal type " + arguments[1]->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);
        // return arguments[1];

        return std::make_shared<DataTypeString>();
    }

    bool useDefaultImplementationForConstants() const override { return true; }

    ColumnNumbers getArgumentsThatAreAlwaysConstant() const override { return {0}; }

    ColumnPtr executeImpl(const ColumnsWithTypeAndName & /*arguments*/, const DataTypePtr &, size_t rows) const override
    {
        // const auto & langcolumn = arguments[0].column;
        // const auto & strcolumn = arguments[1].column;

        // const ColumnConst * lang_col = checkAndGetColumn<ColumnConst>(langcolumn.get());
        // const ColumnString * words_col = checkAndGetColumn<ColumnString>(strcolumn.get());

        // if (!lang_col)
        //     throw Exception(
        //         "Illegal column " + arguments[0].column->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_COLUMN);
        // if (!words_col)
        //     throw Exception(
        //         "Illegal column " + arguments[1].column->getName() + " of argument of function " + getName(), ErrorCodes::ILLEGAL_COLUMN);

        // String language = lang_col->getValue<String>();

        // auto col_res = ColumnString::create();
        // StemImpl::vector(words_col->getChars(), words_col->getOffsets(), col_res->getChars(), col_res->getOffsets(), language);
        // return col_res;
        auto col_res = ColumnString::create();

        ColumnString::Chars & res_data = col_res->getChars();
        ColumnString::Offsets & res_offsets = col_res->getOffsets();

        res_data.resize(rows);
        res_offsets.resize_exact(rows);

        UInt64 data_size = 0;
        for (UInt64 i = 0; i < rows; ++i)
        {
            String res = counter_cuda(20);
            size_t new_size = res.size() + 1;

            if (res_data.size() < data_size + new_size)
                res_data.resize(data_size + new_size);

            memcpy(res_data.data() + data_size, res.data(), res.size());
            res_data.data()[data_size + res.size()] = 0;

            data_size += new_size;
            res_offsets[i] = data_size;
        }
        res_data.resize(data_size);

        return col_res;
    }
};

}

void registerFunctionCuda(FunctionFactory & factory)
{
    factory.registerFunction<FunctionCuda>(FunctionFactory::CaseInsensitive);
}

}

// #endif

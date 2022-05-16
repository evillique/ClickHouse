SELECT key, uniqHLL12(value) FROM test_cuda_big GROUP BY key LIMIT 10 SETTINGS use_cuda_aggregation=0;
SELECT key, uniqHLL12(value) FROM test_cuda_big GROUP BY key LIMIT 10 SETTINGS use_cuda_aggregation=1;
SELECT key, uniqHLL12(value) FROM test_cuda_big GROUP BY key LIMIT 10 SETTINGS use_cuda_aggregation=1;
SELECT key, uniqHLL12(value) FROM test_cuda_big GROUP BY key LIMIT 10 SETTINGS use_cuda_aggregation=1;

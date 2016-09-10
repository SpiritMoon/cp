#!/bin/bash

# 门店日数据统计
python3 /root/cp/db_statistics/store_day_statistics.py	>> /root/day_statistics.log 2>&1

# 客户日数据统计
python3 /root/cp/db_statistics/customer_day_statistics.py	>> /root/day_statistics.log 2>&1

# 市公司日数据统计
python3 /root/cp/db_statistics/C_company_day_statistics.py	>> /root/day_statistics.log 2>&1

# 总的日数据统计
python3 /root/cp/db_statistics/total_day_statistics.py	>> /root/day_statistics.log 2>&1

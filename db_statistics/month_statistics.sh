#!/bin/bash

# 市公司月数据统计
python3 /root/cp/db_statistics/C_company_month_statistics.py	>> /root/month_statistics.log 2>&1

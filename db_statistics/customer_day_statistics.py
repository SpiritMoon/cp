#!/usr/bin/env python3.5
# -*- coding: utf-8 -*-

"""
psycopg2安装
apt-get install libpq-dev
sudo python3.5 -m pip install psycopg2
"""

# 引入psycopg2库
import psycopg2
import time

def conndb():
    """
    连接数据库，并建立Cursor对象
    """
    conn = psycopg2.connect(database="wifi", user="postgres", password="Zzwx13869121158", host="139.129.42.237", port="5432")
    cur = conn.cursor()
    return conn, cur

def create_store_day_stat_row(conn, cur, str_yesterday):
    """
    创建customer_day_statistics的昨日记录
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "create customer_day_statistics recond of %s..." % str_yesterday )
    
    cur.execute("SELECT id FROM customer")
    cids = cur.fetchall()
    for cid in cids:
        # 查询门店的此日期记录在表customer_day_statistics是否存在
        cur.execute("SELECT id FROM customer_day_statistics WHERE c_id = %d AND day = '%s'" % (sid[0], str_yesterday) )
        id = cur.fetchone()
        if id is None:
            # 不存在则插入记录
            cur.execute("""INSERT INTO
                    customer_day_statistics(c_id, day)
                    VALUES(%d, '%s')"""
                    % (cid[0], str_yesterday) )
    return cids

if __name__ == "__main__":
    
    # 开始
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "customer_day_statistics start......")

    # 连接数据库
    conn, cur = conndb()
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "Connect database success!")

    # 打印log表行数
    print_row_num(cur)

    # 开始运行
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "customer_day_statistics running......")
    
    str_yesterday = time.strftime("%Y-%m-%d", time.localtime(time.time() - 24 * 60 * 60))
    
    # 创建customer_day_statistics的昨日记录
    sids = create_store_day_stat_row(conn, cur, str_yesterday)
    print("\n")

    # 统计wifi log
    stat_wifi_log(conn, cur)
    print("\n")

    # 统计新增人数
    stat_new_per_num(conn, cur, str_yesterday, sids)
    print("\n")

    # 统计总上网人数和平均上网时长
    stat_total_per_avg_time(conn, cur)
    print("\n")

    # 统计最近30天的用户上网次数
    stat_last_30_days_conn_num(conn, cur, sids)
    print("\n")

    # 统计结束
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "customer_day_statistics finally......")


    # 关闭Cursor对象和连接对象
    cur.close()
    conn.close()

    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "customer_day_statistics close......")

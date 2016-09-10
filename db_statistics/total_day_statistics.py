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
    conn = psycopg2.connect(database="wifi", user="postgres", password="Zzwx13869121158", host="127.0.0.1", port="5432")
    cur = conn.cursor()
    return conn, cur

def stat_total_day_statistics_for_date(conn, cur, day):
    """
    通过日期和客户id统计客户数据
    """
    print("day is %s" % day)
    cur.execute("""
            SELECT
            (CASE SUM(per_num) WHEN 0 THEN 0 ELSE SUM(per_capita_duration * per_num) / SUM(per_num) END),
            SUM(per_num), 
            SUM(duration_0_10_per_num), SUM(duration_10_30_per_num), 
            SUM(duration_30_60_per_num), SUM(duration_60_120_per_num), 
            SUM(duration_120_240_per_num), SUM(duration_240_max_per_num), 
            SUM(hour_num_0), SUM(hour_num_1), SUM(hour_num_2), SUM(hour_num_3), 
            SUM(hour_num_4), SUM(hour_num_5), SUM(hour_num_6), SUM(hour_num_7), 
            SUM(hour_num_8), SUM(hour_num_9), SUM(hour_num_10), SUM(hour_num_11),
            SUM(hour_num_12), SUM(hour_num_13), SUM(hour_num_14), SUM(hour_num_15),
            SUM(hour_num_16), SUM(hour_num_17), SUM(hour_num_18), SUM(hour_num_19),
            SUM(hour_num_20), SUM(hour_num_21), SUM(hour_num_22), SUM(hour_num_23),
            SUM(new_per_num)
            FROM company_day_statistics
            WHERE day = '%s'
            """
            % day)
    total_data = cur.fetchone()
    print(total_data)
    if total_data[0] is None:
        return

    # 查询客户的此日期记录在表total_day_statistics是否存在
    cur.execute("SELECT id FROM total_day_statistics WHERE day = '%s'" % day)
    tid = cur.fetchone()
    if tid is None:
        # 不存在则插入记录
        cur.execute("""INSERT INTO total_day_statistics
                (day, per_capita_duration, per_num,
                duration_0_10_per_num, duration_10_30_per_num, duration_30_60_per_num,
                duration_60_120_per_num, duration_120_240_per_num, duration_240_max_per_num,
                hour_num_0, hour_num_1, hour_num_2, hour_num_3, hour_num_4, hour_num_5,
                hour_num_6, hour_num_7, hour_num_8, hour_num_9, hour_num_10, hour_num_11,
                hour_num_12, hour_num_13, hour_num_14, hour_num_15, hour_num_16, hour_num_17,
                hour_num_18, hour_num_19, hour_num_20, hour_num_21, hour_num_22, hour_num_23,
                new_per_num)
                VALUES('%s', %0.2f, %d,
                %d, %d, %d,
                %d, %d, %d,
                %d, %d, %d, %d, %d, %d,
                %d, %d, %d, %d, %d, %d,
                %d, %d, %d, %d, %d, %d,
                %d, %d, %d, %d, %d, %d,
                %d)"""
                % ((day,) + total_data) )
    else:
        # 存在则更新记录
        cur.execute("""UPDATE total_day_statistics
                SET per_capita_duration = %0.2f, per_num = %d,
                duration_0_10_per_num = %d, duration_10_30_per_num = %d,
                duration_30_60_per_num = %d, duration_60_120_per_num = %d,
                duration_120_240_per_num = %d, duration_240_max_per_num = %d,
                hour_num_0 = %d, hour_num_1 = %d, hour_num_2 = %d, hour_num_3 = %d,
                hour_num_4 = %d, hour_num_5 = %d, hour_num_6 = %d, hour_num_7 = %d,
                hour_num_8 = %d, hour_num_9 = %d, hour_num_10 = %d, hour_num_11 = %d,
                hour_num_12 = %d, hour_num_13 = %d, hour_num_14 = %d, hour_num_15 = %d,
                hour_num_16 = %d, hour_num_17 = %d, hour_num_18 = %d, hour_num_19 = %d,
                hour_num_20 = %d, hour_num_21 = %d, hour_num_22 = %d, hour_num_23 = %d,
                new_per_num = %d,
                updated_at = CURRENT_TIMESTAMP
                WHERE id = %d"""
                % ( total_data + tid ) )
    conn.commit()





def stat_total_day_statistics(conn, cur, str_yesterday):
    """
    统计客户下的用户上网信息
    先统计昨日的
    然后根据更新时间统计 统计之后有更新的
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "stat wifi log..." )
    
    stat_total_day_statistics_for_date(conn, cur, str_yesterday)
        
    cur.execute("""
                SELECT
                    company_day_statistics. DAY
                FROM
                    company_day_statistics,
                    total_day_statistics
                WHERE
                    company_day_statistics. DAY = total_day_statistics. DAY
                AND company_day_statistics.updated_at > total_day_statistics.updated_at
                GROUP BY
                    company_day_statistics. DAY
                """
                )
    days = cur.fetchall()
    for day in days:
        stat_total_day_statistics_for_date(conn, cur, day[0])
        
        
def stat_company_table(conn, cur):
    """
    统计存放在company表中的统计数据：平均在线市场，总认证人数，最近30天的上网次数
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "stat company table..." )
    
    cur.execute("""
        SELECT per_capita_duration, per_num, last30_conn_num_stat
        FROM company
        WHERE p_id = 0"""
    )
    companies_data = cur.fetchall()
        
    # 处理后的数据存放变量
    per_capita_duration = 0.0
    per_num = 0
    last30_1 = 0
    last30_2 = 0
    last30_5 = 0
    last30_10 = 0
        
    # 处理数据
    for company_data in companies_data:
        print(company_data)
        per_capita_duration += (company_data[0] * company_data[1])
        per_num += company_data[1]
            
        # 切出来最近30天的上网次数数据
        last30_conn_num_stat = company_data[2].split(',')
            
        last30_1 += int(last30_conn_num_stat[0])
        last30_2 += int(last30_conn_num_stat[1])
        last30_5 += int(last30_conn_num_stat[2])
        last30_10 += int(last30_conn_num_stat[3])

    # 计算平均在线时长
    if per_num != 0:
        per_capita_duration /= per_num
    else:
        per_capita_duration = 0
        
    print("per_capita_duration = %0.2f, per_num = %d, last30_conn_num_stat = '%d,%d,%d,%d'"
            %(per_capita_duration, per_num, last30_1, last30_2, last30_5, last30_10) )
        
    # 更新到表中
    cur.execute("""
            UPDATE total_statistics
            SET per_capita_duration = %0.2f,
            per_num = %d,
            last30_conn_num_stat = '%d,%d,%d,%d',
            updated_at = CURRENT_TIMESTAMP
            """
            %(per_capita_duration, per_num, last30_1, last30_2, last30_5, last30_10) )
    conn.commit()

if __name__ == "__main__":
    
    # 开始
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "total_day_statistics start......")

    # 连接数据库
    conn, cur = conndb()
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "Connect database success!")

    # 开始运行
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "total_day_statistics running......")
    str_yesterday = time.strftime("%Y-%m-%d", time.localtime(time.time() - 24 * 60 * 60))
    
    # 统计wifi log
    stat_total_day_statistics(conn, cur, str_yesterday)
    print("\n")

    # 统计总上网人数和平均上网时长 最近30天的用户上网次数
    stat_company_table(conn, cur)
    print("\n")

    # 统计结束
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "total_day_statistics finally......")


    # 关闭Cursor对象和连接对象
    cur.close()
    conn.close()

    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "total_day_statistics close......")

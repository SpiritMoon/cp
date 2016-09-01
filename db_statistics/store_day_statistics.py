#!/usr/bin/env python3.5
# -*- coding: utf-8 -*-

# psycopg2安装
# sudo python3.5 -m pip install psycopg2

# 引入psycopg2库
import psycopg2
import time

print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics start......")


# 连接到数据库test
conn = psycopg2.connect(database="wifi", user="postgres", password="Zzwx13869121158", host="139.129.42.237", port="5432")
# 建立Cursor对象
cur = conn.cursor()

print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "Connect database success!")

cur.execute("SELECT COUNT(id) FROM wifi_user_log")
count = cur.fetchone()
print("wifi_user_log count:", count)
cur.execute("SELECT COUNT(id) FROM wifi_user_log_backup")
count = cur.fetchone()
print("wifi_user_log_backup count:", count)

print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics running......")

# 将数据按照门店id 和 日期进行分组，并统计出平均在线时间和用户数
cur.execute("""SELECT s_id, EXTRACT(YEAR FROM conn_time), EXTRACT(MONTH FROM conn_time), EXTRACT(DAY FROM conn_time), AVG(online_time), COUNT(id)
        FROM wifi_user_log
        WHERE complete = True AND EXTRACT(DAY FROM conn_time) != EXTRACT(DAY FROM CURRENT_TIMESTAMP)
        GROUP BY s_id, EXTRACT(YEAR FROM conn_time), EXTRACT(MONTH FROM conn_time), EXTRACT(DAY FROM conn_time)
        ORDER BY s_id, EXTRACT(YEAR FROM conn_time), EXTRACT(MONTH FROM conn_time), EXTRACT(DAY FROM conn_time)
        """)
stores = cur.fetchall()

# 逐个处理每个分组
for store in stores:
    print( "%d:%d-%02d-%02d -- avg = %0.2f, count(id) = %d" % store )

    # 计算每个上网时长(0-10,10-30,30-60,60-120,120-240,240+)的用户数
    ot_duration_group = [0, 0, 0, 0, 0, 0]
    cur.execute("""SELECT MIN(online_time), COUNT(id) FROM wifi_user_log
            WHERE complete = TRUE AND s_id = %d
            AND EXTRACT(YEAR FROM conn_time) = %d AND EXTRACT(MONTH FROM conn_time) = %d AND EXTRACT(DAY FROM conn_time) = %d
            GROUP BY online_time < 10, online_time >= 10 AND online_time < 30, online_time >= 30 AND online_time < 60,
            online_time >= 60 AND online_time < 120, online_time >= 120 AND online_time < 240, online_time >= 240"""
            % store[0:4])
    ots = cur.fetchall()
    print("online time duration group:", ots)
    for ot in ots:
        if ot[0] < 10:
            ot_duration_group[0] = ot[1]
        elif ot[0] >= 10 and ot[0] < 30:
            ot_duration_group[1] = ot[1]
        elif ot[0] >= 30 and ot[0] < 60:
            ot_duration_group[2] = ot[1]
        elif ot[0] >= 60 and ot[0] < 120:
            ot_duration_group[3] = ot[1]
        elif ot[0] >= 120 and ot[0] < 240:
            ot_duration_group[4] = ot[1]
        else:
            ot_duration_group[5] = ot[1]
    
    print("online time duration group:", ot_duration_group)

    # 计算每个小时连接用户数
    conn_hour_num = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
    cur.execute("""SELECT EXTRACT(HOUR FROM conn_time), count(id) FROM wifi_user_log
            WHERE complete = TRUE AND s_id = %d
            AND EXTRACT(YEAR FROM conn_time) = %d AND EXTRACT(MONTH FROM conn_time) = %d AND EXTRACT(DAY FROM conn_time) = %d
            GROUP BY EXTRACT(HOUR FROM conn_time)"""
            % store[0:4])
    conn_hours = cur.fetchall()
    print("conn time group hour:", conn_hours)
    for conn_hour in conn_hours:
        conn_hour_num[ int(conn_hour[0]) ] = conn_hour[1]

    print("conn time group hour:", conn_hour_num)
    

    # 查询门店的此日期记录是否存在
    cur.execute("SELECT id FROM store_day_statistics WHERE s_id = %d AND day = '%d-%02d-%02d'" % store[0:4])
    id = cur.fetchone()
    if id is None:
        # 不存在则插入记录
        cur.execute("""INSERT INTO store_day_statistics
                (s_id, day, per_capita_duration, per_num,
                duration_0_10_per_num, duration_10_30_per_num, duration_30_60_per_num, duration_60_120_per_num, duration_120_240_per_num, duration_240_max_per_num,
                hour_num_0, hour_num_1, hour_num_2, hour_num_3, hour_num_4, hour_num_5, hour_num_6, hour_num_7, hour_num_8, hour_num_9, hour_num_10, hour_num_11,
                hour_num_12, hour_num_13, hour_num_14, hour_num_15, hour_num_16, hour_num_17, hour_num_18, hour_num_19, hour_num_20, hour_num_21, hour_num_22, hour_num_23)
                VALUES(%d, '%d-%02d-%02d', %0.2f, %d,
                %d, %d, %d, %d, %d, %d,
                %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,
                %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)"""
                % (store + tuple(ot_duration_group) + tuple(conn_hour_num)) )
    else:
        # 存在则更新记录
        cur.execute("""UPDATE store_day_statistics
                SET per_capita_duration = (per_capita_duration * per_num + %d * %d)/(per_num + %d),
                per_num = per_num + %d,
                duration_0_10_per_num = duration_0_10_per_num + %d,
                duration_10_30_per_num = duration_10_30_per_num + %d,
                duration_30_60_per_num = duration_30_60_per_num + %d,
                duration_60_120_per_num = duration_60_120_per_num + %d,
                duration_120_240_per_num = duration_120_240_per_num + %d,
                duration_240_max_per_num = duration_240_max_per_num + %d,
                hour_num_0 = hour_num_0 + %d,
                hour_num_1 = hour_num_1 + %d,
                hour_num_2 = hour_num_2 + %d,
                hour_num_3 = hour_num_3 + %d,
                hour_num_4 = hour_num_4 + %d,
                hour_num_5 = hour_num_5 + %d,
                hour_num_6 = hour_num_6 + %d,
                hour_num_7 = hour_num_7 + %d,
                hour_num_8 = hour_num_8 + %d,
                hour_num_9 = hour_num_9 + %d,
                hour_num_10 = hour_num_10 + %d,
                hour_num_11 = hour_num_11 + %d,
                hour_num_12 = hour_num_12 + %d,
                hour_num_13 = hour_num_13 + %d,
                hour_num_14 = hour_num_14 + %d,
                hour_num_15 = hour_num_15 + %d,
                hour_num_16 = hour_num_16 + %d,
                hour_num_17 = hour_num_17 + %d,
                hour_num_18 = hour_num_18 + %d,
                hour_num_19 = hour_num_19 + %d,
                hour_num_20 = hour_num_20 + %d,
                hour_num_21 = hour_num_21 + %d,
                hour_num_22 = hour_num_22 + %d,
                hour_num_23 = hour_num_23 + %d
                WHERE id = %d"""
                % (store[4:6] + store[5:6] + store[5:6] + tuple(ot_duration_group) + tuple(conn_hour_num) + tuple(id)) )

    # 查询出已统计的记录
    cur.execute("""SELECT wu_id, ip, apid, login_type, complete, conn_time, disconn_time, online_time, s_id
            FROM wifi_user_log WHERE s_id = %d AND complete = True
            AND EXTRACT(YEAR FROM conn_time) = %d AND EXTRACT(MONTH FROM conn_time) = %d AND EXTRACT(DAY FROM conn_time) = %d"""
            % store[0:4])
    reconds = cur.fetchall()
    # 保存到backup表
    for recond in reconds:
        cur.execute("""INSERT INTO wifi_user_log_backup(wu_id, ip, apid, login_type, complete, conn_time, disconn_time, online_time, s_id, created_at)
                VALUES(%d, '%s', %d, '%c', '%s', '%s', '%s', %d, %d, CURRENT_TIMESTAMP)"""
                % recond)
    #删除数据
    cur.execute("""DELETE FROM wifi_user_log
            WHERE s_id = %d AND complete = True
            AND EXTRACT(YEAR FROM conn_time) = %d AND EXTRACT(MONTH FROM conn_time) = %d AND EXTRACT(DAY FROM conn_time) = %d"""
            % store[0:4])
    
    # 提交数据改变
    conn.commit()

    cur.execute("SELECT COUNT(id) FROM wifi_user_log")
    count = cur.fetchone()
    print("wifi_user_log count:", count)
    cur.execute("SELECT COUNT(id) FROM wifi_user_log_backup")
    count = cur.fetchone()
    print("wifi_user_log_backup count:", count)

print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics finally......")

cur.execute("SELECT COUNT(id) FROM wifi_user_log")
count = cur.fetchone()
print("wifi_user_log count:", count)
cur.execute("SELECT COUNT(id) FROM wifi_user_log_backup")
count = cur.fetchone()
print("wifi_user_log_backup count:", count)

# 关闭Cursor对象和连接对象
cur.close()
conn.close()

print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics close......")

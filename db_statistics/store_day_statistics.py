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

def print_row_num(cur):
    """
    打印log表行数
    """
    print("\n* * * * * * Row Num * * * * * *")
    cur.execute("SELECT COUNT(id) FROM wifi_user_log")
    count = cur.fetchone()
    print("wifi_user_log count: ", count[0])
    cur.execute("SELECT COUNT(id) FROM wifi_user_log_backup")
    count = cur.fetchone()
    print("wifi_user_log_backup count: ", count[0])
    print("* * * * * * Row Num * * * * * *\n")



def create_store_day_stat_row(conn, cur, str_yesterday):
    """
    创建store_day_statistics的昨日记录
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "create store_day_statistics recond of %s..." % str_yesterday )
    
    cur.execute("SELECT id FROM store")
    sids = cur.fetchall()
    for sid in sids:
        # 查询门店的此日期记录在表store_day_statistics是否存在
        cur.execute("SELECT id FROM store_day_statistics WHERE s_id = %d AND day = '%s'" % (sid[0], str_yesterday) )
        id = cur.fetchone()
        if id is None:
            # 不存在则插入记录
            cur.execute("""INSERT INTO store_day_statistics(s_id, day)
                    VALUES(%d, '%s')"""
                    % (sid[0], str_yesterday) )
    conn.commit()
    return sids

def stat_wifi_log(conn, cur):
    """
    从wifi_user_log表中分组查找出今日之前的已经完结记录
    统计在线用户数、平均上网时间、上网时间分段用户数、各时段上线人数
    然后插入或者更新到store_day_statistics表中
    操作完成后，将已统计的数据转移到备份数据表中
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "select store id and day in wifi_user_log...")
    
    # 按照门店id 和 日期将表wifi_user_log进行分组，并统计出平均在线时间和用户数
    cur.execute("""SELECT s_id, EXTRACT(YEAR FROM conn_time), EXTRACT(MONTH FROM conn_time), EXTRACT(DAY FROM conn_time), AVG(online_time), COUNT(id)
            FROM wifi_user_log
            WHERE complete = True AND EXTRACT(DAY FROM conn_time) != EXTRACT(DAY FROM CURRENT_TIMESTAMP)
            GROUP BY s_id, EXTRACT(YEAR FROM conn_time), EXTRACT(MONTH FROM conn_time), EXTRACT(DAY FROM conn_time)
            ORDER BY s_id, EXTRACT(YEAR FROM conn_time), EXTRACT(MONTH FROM conn_time), EXTRACT(DAY FROM conn_time)""")
    stores = cur.fetchall()
    
    # 逐个处理每个分组
    for store in stores:
        print( "\n%d:%d-%02d-%02d -- avg = %0.2f, count(id) = %d" % store )

        # 计算每个上网时长(0-10,10-30,30-60,60-120,120-240,240+)的用户数
        ot_duration_group = [0, 0, 0, 0, 0, 0]
        cur.execute("""SELECT MIN(online_time), COUNT(id) FROM wifi_user_log
                WHERE complete = TRUE AND s_id = %d
                AND EXTRACT(YEAR FROM conn_time) = %d AND EXTRACT(MONTH FROM conn_time) = %d AND EXTRACT(DAY FROM conn_time) = %d
                GROUP BY online_time < 10, online_time >= 10 AND online_time < 30, online_time >= 30 AND online_time < 60,
                online_time >= 60 AND online_time < 120, online_time >= 120 AND online_time < 240, online_time >= 240"""
                % store[0:4])
        ots = cur.fetchall()
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
        for conn_hour in conn_hours:
            conn_hour_num[ int(conn_hour[0]) ] = conn_hour[1]

        print("conn time group hour:", conn_hour_num)
    

        # 查询门店的此日期记录在表store_day_statistics是否存在
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
                    hour_num_23 = hour_num_23 + %d,
                    updated_at = CURRENT_TIMESTAMP
                    WHERE id = %d"""
                    % (store[4:6] + store[5:6] + store[5:6] + tuple(ot_duration_group) + tuple(conn_hour_num) + tuple(id)) )

        # 将已统计过的数据转存到backup表
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
        
        # 打印当前行数
        print_row_num(cur)

def stat_new_per_num(conn, cur, str_yesterday, sids):
    """
    统计昨日的新增人数
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "select %s new per num..." % str_yesterday )
    
    for sid in sids:
        # 查询出没有数据的当天新增的用户数量
        cur.execute("""SELECT COUNT(id)
                FROM wifi_user
                WHERE s_id = %d AND created_at >= '%s 0:0:0' AND created_at <= '%s 23:59:59'"""
                % (sid[0], str_yesterday, str_yesterday) )
        new_per_num = cur.fetchone()

        print("s_id %d -- %d" % (sid[0], new_per_num[0]))

        # 更新到数据表中
        cur.execute("""UPDATE store_day_statistics
                SET new_per_num = %d
                WHERE s_id = %d AND day = '%s'"""
                % (new_per_num[0], sid[0], str_yesterday) )
    conn.commit()

def stat_total_per_avg_time(conn, cur):
    """
    统计总的上网人数和平均上网时间更新到store表
    """
    
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "select total conn per num and avg conn time ..." )
    
    cur.execute("""SELECT SUM(per_num), SUM(per_capita_duration * per_num), s_id
            FROM store_day_statistics
            GROUP BY s_id""")
    stores = cur.fetchall()
    for store in stores:
        
        if store[0] == 0:
            print("sid %d: 0 -- 0" % store[2])
            cur.execute("""UPDATE store
                    SET per_num = %d, per_capita_duration = %d
                    WHERE id = %d"""
                    % (0, 0, store[2]))
        else:
            print("sid %d: %d -- %d" % (store[2], store[1] / store[0], store[0]))
            cur.execute("""UPDATE store
                    SET per_num = %d, per_capita_duration = %d
                    WHERE id = %d"""
                    % (store[0], store[1] / store[0], store[2]) )
    conn.commit()

def stat_last_30_days_conn_num(conn, cur, sids):
    """
    统计最近30天，用户的上网次数
    """
    
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "select last 30 days per conn num ..." )
    
    # 遍历出门店列表
    for sid in sids:
        
        conn_num_list = [0, 0, 0, 0]
        
        # 查出用户上网次数
        cur.execute("""SELECT MIN(num), COUNT(num) FROM 
                (SELECT COUNT(id) AS num
                FROM wifi_user_log_backup
                WHERE s_id = %d AND conn_time >= CURRENT_TIMESTAMP - INTERVAL '31 Days'
                GROUP BY wu_id
                ) AS nums
                GROUP BY num = 1, num >1 AND num < 5, num >=5 AND num < 10, num > 10"""
                % (sid[0]) )
        conn_nums = cur.fetchall()
        for conn_num in conn_nums:
            if conn_num[0] == 1:
                conn_num_list[0] = conn_num[1]
            elif conn_num[0] < 5:
                conn_num_list[1] = conn_num[1]
            elif conn_num[0] < 10:
                conn_num_list[2] = conn_num[1]
            else:
                conn_num_list[3] = conn_num[1]

        print("s_id %d --" % sid[0], conn_num_list)
        # 更新到数据表中
        cur.execute("""UPDATE store
                SET last30_conn_num_stat = '%d,%d,%d,%d'
                WHERE id = %d"""
                % (tuple(conn_num_list) + sid) )
    conn.commit()
    

if __name__ == "__main__":
    
    # 开始
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics start......")

    # 连接数据库
    conn, cur = conndb()
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "Connect database success!")

    # 打印log表行数
    print_row_num(cur)

    # 开始运行
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics running......")
    
    str_yesterday = time.strftime("%Y-%m-%d", time.localtime(time.time() - 24 * 60 * 60))
    
    # 创建store_day_statistics的昨日记录
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
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics finally......")


    # 关闭Cursor对象和连接对象
    cur.close()
    conn.close()

    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "store_day_statistics close......")

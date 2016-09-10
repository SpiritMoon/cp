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

def get_cids(cur):
    """
    查询出 顶级company id的列表
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "get cids...")

    # 存放id对应关系的字典
    cur.execute("SELECT id FROM company WHERE p_id = 0")
    cids = cur.fetchall()
    print(cids)
    return cids

def get_lastmonth(cur):
    cur.execute("""
            SELECT
                EXTRACT (YEAR FROM CURRENT_TIMESTAMP - INTERVAL '1 MONTH'),
                EXTRACT (MONTH FROM CURRENT_TIMESTAMP - INTERVAL '1 MONTH')
            """)
    ym_date = cur.fetchone()
    return ym_date

def stat_new_customer_num(cur, cid, lm_year, lm_month):
    cur.execute("""
            SELECT
                COUNT (ID)
            FROM
                customer
            WHERE
                company_id IN (
                    SELECT
                        "id"
                    FROM
                        company
                    WHERE
                        p_id = %d
                    OR
                        ID = %d
                )
            AND EXTRACT (YEAR FROM created_at) = %d
            AND EXTRACT (MONTH FROM created_at) = %d
            """
            % (cid, cid, lm_year, lm_month))
    new_customer_num = cur.fetchone()
    return new_customer_num[0]

def stat_new_store_num(cur, cid, lm_year, lm_month):
    cur.execute("""
            SELECT
                COUNT (ID)
            FROM
                store
            WHERE
                customer_id IN (
                    SELECT ID FROM customer WHERE company_id IN (
                        SELECT ID FROM company WHERE p_id = %d OR ID = %d
                    )
                )
            AND EXTRACT (YEAR FROM created_at) = %d
            AND EXTRACT (MONTH FROM created_at) = %d
            """
            %
            (cid, cid, lm_year, lm_month)
    )
    new_store_num = cur.fetchone()
    return new_store_num[0]

def stat_new_cw_num(cur, cid, lm_year, lm_month):
    cur.execute("""
            SELECT
                COUNT (ID)
            FROM
                customer_workorder
            WHERE
                customer_id IN (
                    SELECT ID FROM customer WHERE company_id IN (
                        SELECT ID FROM company WHERE p_id = %d OR ID = %d
                    )
                )
            AND EXTRACT (YEAR FROM created_at) = % d
            AND EXTRACT (MONTH FROM created_at) = % d
            """
            %
            (cid, cid, lm_year, lm_month)
            )
    new_cw_num = cur.fetchone()
    return new_cw_num[0]

def stat_new_ap_num(cur, cid, lm_year, lm_month):
    cur.execute("""
            SELECT
                COUNT(id)
            FROM
                ap
            WHERE
                wr_id IN (
                    SELECT ID FROM customer_workorder WHERE customer_id IN (
                        SELECT ID FROM customer WHERE company_id IN (
                            SELECT ID FROM company WHERE p_id = %d OR ID = %d
                        )
                    )
                )
            AND EXTRACT (YEAR FROM created_at) = % d
            AND EXTRACT (MONTH FROM created_at) = % d
            """
            %
            (cid, cid, lm_year, lm_month)
            )
    new_ap_num = cur.fetchone()
    return new_ap_num[0]

def stat_optimal_customer_manager(cur, cid, lm_year, lm_month):
    cur.execute("""
            SELECT
                COUNT (ap. ID) AS apnum,
                users.realname,
                users. ID
            FROM
                ap,
                customer_workorder,
                users
            WHERE
                ap.wr_id = customer_workorder. ID
            AND customer_workorder.cm_u_id = users."id"
            AND EXTRACT (YEAR FROM ap.created_at) = %d
            AND EXTRACT (MONTH FROM ap.created_at) = %d
            AND users.company_id IN (
                SELECT ID FROM company WHERE p_id = %d OR ID = %d
                )
            GROUP BY
                users."id"
            ORDER BY
                apnum DESC
            LIMIT 1
            """
            %
            (lm_year, lm_month, cid, cid)
            )
    optimal_customer_manager = cur.fetchone()
    if optimal_customer_manager is None:
        return 0
    else:
        print("%d-%02d 月份 optimal_customer_manager -- " % (lm_year, lm_month), optimal_customer_manager )
        return optimal_customer_manager[2]

def stat_optimal_branch_company(cur, cid, lm_year, lm_month):
    cur.execute("""
            SELECT
                COUNT(ap. ID) as apnum,
                company."name",
                company."id"
            FROM
                ap,
                store,
                customer,
                company
            WHERE
                ap.s_id = store."id"
            AND store.customer_id = customer."id"
            AND customer.company_id = company."id"
            AND EXTRACT (YEAR FROM ap.created_at) = %d
            AND EXTRACT (MONTH FROM ap.created_at) = %d
            AND company."id" IN (
                SELECT ID FROM company WHERE p_id = %d OR ID = %d
                )
            GROUP BY
                company."id"
            ORDER BY
                apnum DESC
            LIMIT 1
            """
            %
            (lm_year, lm_month, cid, cid)
            )
    optimal_branch_company = cur.fetchone()
    if optimal_branch_company is None:
        return 0
    else:
        print("%d-%02d 月份 optimal_branch_company -- " % (lm_year, lm_month), optimal_branch_company )
        return optimal_branch_company[2]

def stat_company_month(conn, cur, cids, lm_year, lm_month):
    """
    统计存放在customer表中的统计数据：平均在线市场，总认证人数，最近30天的上网次数
    """
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "stat company month..." )
    
    for cid in cids:
        print("cid is %d" % cid[0])
        # 客户
        new_customer_num = stat_new_customer_num(cur, cid[0], lm_year, lm_month)
        print("%d-%02d 月份 new_customer_num -- %d" % (lm_year, lm_month, new_customer_num) )
        
        # 门店
        new_store_num = stat_new_store_num(cur, cid[0], lm_year, lm_month)
        print("%d-%02d 月份 new_store_num -- %d" % (lm_year, lm_month, new_store_num) )

        # 工单
        new_cw_num = stat_new_cw_num(cur, cid[0], lm_year, lm_month)
        print("%d-%02d 月份 new_cw_num -- %d" % (lm_year, lm_month, new_cw_num) )
        
        # ap
        new_ap_num = stat_new_ap_num(cur, cid[0], lm_year, lm_month)
        print("%d-%02d 月份 new_ap_num -- %d" % (lm_year, lm_month, new_ap_num) )

        # 最佳经理
        optimal_customer_manager = stat_optimal_customer_manager(cur, cid[0], lm_year, lm_month)

        # 最佳公司
        optimal_branch_company = stat_optimal_branch_company(cur, cid[0], lm_year, lm_month)
       
        # 查询记录是否存在 补存在插入 存在更新
        cur.execute("SELECT ID FROM company_month_statistics WHERE company_id = %d AND month = '%d-%02d'"
                % (cid[0], lm_year, lm_month))
        cmsid = cur.fetchone()
        if cmsid is None:
            cur.execute("""
                    INSERT INTO company_month_statistics
                    (
                        company_id, month, new_customer_num, new_store_num,
                        new_ap_num, new_cw_num, optimal_customer_manager, optimal_branch_company
                    )
                    VALUES
                    (
                        %d, '%d-%02d', %d, %d,
                        %d, %d, %d, %d
                    )
                    """
                    %
                    (cid[0], lm_year, lm_month, new_customer_num, new_store_num,
                    new_ap_num, new_cw_num, optimal_customer_manager, optimal_branch_company)
            )
        else:
            cur.execute("""
                    UPDATE company_month_statistics
                    SET
                        new_customer_num = %d, new_store_num = %d, new_ap_num = %d,
                        new_cw_num = %d, optimal_customer_manager = %d, optimal_branch_company = %d,
                        updated_at = CURRENT_TIMESTAMP
                    WHERE
                        id = %d
                    """
                    %
                    (new_customer_num, new_store_num, new_ap_num,
                    new_cw_num, optimal_customer_manager, optimal_branch_company,
                    cmsid[0])
            )
        conn.commit()

if __name__ == "__main__":
    
    # 开始
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "company_day_statistics start......")

    # 连接数据库
    conn, cur = conndb()
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "Connect database success!")

    # 开始运行
    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "company_day_statistics running......")
     
    # 获取顶级市公司id
    cids = get_cids(cur)
    print()
    
    # 获取上月的年份和月份
    lm_year, lm_month = get_lastmonth(cur)
    print("Get last month is %d-%02d\n" % (lm_year, lm_month) )

    # 统计上月的数据
    stat_company_month(conn, cur, cids, lm_year, lm_month)
    

    # 统计结束
    print(time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "company_day_statistics finally......")


    # 关闭Cursor对象和连接对象
    cur.close()
    conn.close()

    print( time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()), " -- ", "company_day_statistics close......")

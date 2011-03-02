#include <sys/types.h>
#include <stdio.h>
#include <db.h>

#define DATABASE "/riot/access.db"


int
main()
{
    DB *dbp;
    DBT key, data;
    int ret, t_ret;


    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        fprintf(stderr, "db_create: %s\n", db_strerror(ret));
        exit (1);
    }
    if ((ret = dbp->open(dbp,
                    NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
        dbp->err(dbp, ret, "%s", DATABASE);
        goto err;
    }


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    double a = 1.0;
    for (int i=0; i<1;++i) {
        for (int j=0; j<1; ++j) {
            int k = i*1024+j;
    key.data = &k;
    key.size = sizeof(int);
    data.data = &a;
    data.size = sizeof(double);


    if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) == 0)
        ;//printf("db: %s: key stored.\n", (char *)key.data);
    else {
        dbp->err(dbp, ret, "DB->put");
        goto err;
    }
        }}


    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    int k = 0;
    key.data = &k;
    key.size = sizeof(int);
    data.size = sizeof(double);
    data.data = &a;
    data.ulen = sizeof(double);
    data.flags = DB_DBT_USERMEM;
    if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0)
        printf("db: %d: key retrieved: data was %f.\n",
                *(int *)key.data, *(double *)data.data);
    else {
        dbp->err(dbp, ret, "DB->get");
        goto err;
    }


err:    if ((t_ret = dbp->close(dbp, 0)) != 0 && ret == 0)
            ret = t_ret; 


        exit(ret);
}

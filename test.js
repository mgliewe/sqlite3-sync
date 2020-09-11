
let Drv = require('./index');
let db = new Drv('catalog.sqlite');
let format = require('sprintf-js').sprintf;

let years = db.all( "select distinct strftime('%Y', date) as year " +
                    " from shop_carts " +
                    " order by year", []);
for (let yi in years) {
    let year = years[yi].year;

    console.log('Journal for '+year);
    console.log('================\n');

    console.log('by month:')
    console.log('        prepaid        cash         sum');
    console.log('      ---------------------------------')
    let months = db.all("select distinct strftime('%m', date) as month " + 
                        " from shop_carts "+
                        " where strftime('%Y', date) = ? "+
                        " order by month" , [year]);
    for (let mi in months) {
        let month = months[mi].month;
        let pp=0, cash=0
        db.each(
            "select total, payment "+
            " from shop_carts "+
            " where strftime('%Y', date) = ? and strftime('%m', date) = ?" , [ year, month ],
            (row) => {
                if (row.payment=='cash') {
                    cash += row.total;
                }
                if (row.payment=='prepaid') {
                    pp += row.total;
                }
            }, () => {
                pp = Math.round(pp*100)/100;
                cash = Math.round(cash*100)/100;
                let sum = Math.round((cash+pp)*100)/100
                console.log(format("%' 2d  %' 10.2f€ %' 10.2f€ %' 10.2f€", month, pp, cash, sum));
            }
        );
    }
    
    console.log('\nby week:')
    let weeks = db.all("select distinct strftime('%W', date)+1 as week " + 
                        " from shop_carts "+
                        " where strftime('%Y', date) = ? "+
                        " order by week" , [year]);
    for (wi in weeks) {
        let week = weeks[wi].week;
        let pp=0, cash=0
        db.each(
            "select total, payment "+
            " from shop_carts "+
            " where strftime('%Y', date) = ? and strftime('%W', date)+1 = ?" , [ year, week ],
            (row) => {
                if (row.payment=='cash') {
                    cash += row.total;
                }
                if (row.payment=='prepaid') {
                    pp += row.total;
                }
            }, () => {
                pp = Math.round(pp*100)/100;
                cash = Math.round(cash*100)/100;
                let sum = Math.round((cash+pp)*100)/100
                console.log(format("%' 2d  %' 10.2f€ %' 10.2f€ %' 10.2f€", week, pp, cash, sum));
            }
        );
    }


    console.log('\n\n');
}
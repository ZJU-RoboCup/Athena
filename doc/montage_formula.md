* The areas in the filed are divided into three parts：
    * 1. once dected
    * 2. twice dected
    * 3. four-times dected
> refering to the picture below.
>
> 电脑上git上传到新建github项目

![image](https://github.com/panrusheng/md_pic/raw/master/field.jpg)

* The expression of ball position are shown respectively:
    * symbols description:

        > In zone_2, `d` stands for the vertical width of the strip domain

        > In zone_2, `d_k` (k can be a,b,c or d) stands for the vertical length from area_k to the ball

        > In zone_3, `c_k` (k can be a,b,c or d) stands for the diagnoal length from the angle of area_k to the ball

    * zone 1: `ball.pos` = `ball.pos_a`

    * zone 2: `ball.pos` = [`ball.pos_a` * (`d` - `d_a`) + `ball.pos_b` * `d_a`]/`d`   

    * zone 3: `ball.pos` = [`ball.pos_a` * `(1.414d - c_a)` + `ball.pos_b` * `(1.414d - c_b)` + `ball.pos_c` * `(1.414d - c_c)` + `ball.pos_d` * `(1.414d - c_d)`]/[`5.657d`-`(d_a+d_b+d_c+d_d)`] 
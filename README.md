# hellcopter
Battlebots thing that I did with logan.

We made a jank battlebots with a 150gram weight limit. It was controlled by BLE because we were poor and didn't wanna buy a radio controller. It worked pretty well.
A python scripted connected to the ESP32 and sent continuous simple commands such as: forward, backward, left, right, weapon, and boost. 
This made full use of the BLE byte limit.

Moment of intertia compensation was also implemented because turning on/off the weapon motor made the thing turn left/right and made the whole thing unstable. that worked well.
Main flaw was the high COM. Made the thing tip over when it hit something/or rode on a bumpy surface.

safety stuff was also implemented but that's boring.

Hardware description with cost


| Component          | Model / Type / Spec              | Link                                                                                                                                                           | Cost (CAD)    | Status           | Notes                                                                  |
| ------------------ | -------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------- | ---------------- | ---------------------------------------------------------------------- |
| **Drive Motors**   | N20 Gear Motor                   | [https://www.canadarobotix.com/products/2839](https://www.canadarobotix.com/products/2839)                                                                     | 40.46         | Need to purchase | Higher torque, slower speed. Meets torque needs (likely).              |
| **Weapon Motor**   | Brushless ~2204 size, >100KV     | [https://www.aliexpress.com/item/1005007472524964.html](https://www.aliexpress.com/item/1005007472524964.html)                                                 | 1.38          | Need to purchase | Anything will do, just ensure KV >100 and proper size/weight.          |
| **Weapon ESC**     | BLHeli-S 20A Brushless           | [https://www.aliexpress.com/item/1005002742607987.html](https://www.aliexpress.com/item/1005002742607987.html)                                                 | 9.39          | **Ordered**      | From AliExpress.                                                       |
| **Battery**        | Fullymax 2.2Wh 7.4V              | *(n/a)*                                                                                                                                                        | Free          | **Have**         | Coworker generously donated                                    |
| **Power Switch**   | Generic                          | *(n/a)*                                                                                                                                                        | —             | Need to source   | —                                                                      |
| **Status LED**     | Generic                          | *(n/a)*                                                                                                                                                        | Free          | **Have**         | —                                                                      |
| **Motor Driver**   | DRV8833                          | [https://www.amazon.ca/dp/B0DDCFMJGC](https://www.amazon.ca/dp/B0DDCFMJGC)                                                                                     | 11.29         | Need to purchase | For drive motors if using brushed configuration.                       |
| **Mini MCU**       | Xiao ESP32S3                     | *(n/a)*                                                                                                                                                        | Free          | **Have**         | Enables semi-autonomous option. Camera available if we want challenge. |
| **Buck Converter** | Mini-360 (lightweight regulator) | [https://www.amazon.ca/Willwin-Converter-Module-4-75V-23V-1V-17V/dp/B076KLFQNS](https://www.amazon.ca/Willwin-Converter-Module-4-75V-23V-1V-17V/dp/B076KLFQNS) | 17.99 → 20.33 | Need to purchase | Step-down for stable logic voltage.                                    |




here's a picture of the cad model:
<img width="915" height="662" alt="image" src="https://github.com/user-attachments/assets/c11d581a-e3f3-4e11-8bdd-6bbfff57597d" />


Here's the actual thing:
<img width="3024" height="4032" alt="image" src="https://github.com/user-attachments/assets/19ac0008-fb1f-4c60-bea8-3e421498c009" />


I wish i had a video of the thing but i don't know how to put that in so trust me bro.





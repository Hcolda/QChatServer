### 登录接口
**接口定义：**
- 请求终端：`客户端`
- 请求方式：`POST`
- 请求链接：`https://testapi.hcolda.com/api.php?type=application&commend=login`

**请求参数列表：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|username|是|string|示例值:`Name`|
|password|是|string|示例值:`PWS123`|

**结果字段说明：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|state|是|string|示例值:`success`|
|uuid|是|string|示例值:`225BD9F9-493D-F9E5-50A4-9BE0C59FB859`|
|token|是|string|示例值:`r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x...`|

**原始数据：**
```json
{
    "state":"success",
    "uuid":"225BD9F9-493D-F9E5-50A4-9BE0C59FB859",
    "token":"r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x4qhDd3J08HtHVj"
}
```

### 注册接口
**接口定义：**
- 请求终端：`客户端`
- 请求方式：`POST`
- 请求链接：`https://testapi.hcolda.com/api.php?type=application&commend=signup`

**请求参数列表：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|username|是|string|示例值:`Name`|
|password|是|string|示例值:`PWS123`|
|email|是|string|示例值:`abc@hcolda.com`|

**结果字段说明：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|state|是|string|示例值:`success`|
|uuid|是|string|示例值:`225BD9F9-493D-F9E5-50A4-9BE0C59FB859`|
|token|是|string|示例值:`r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x...`|

**原始数据：**
```json
{
    "state":"success",
    "uuid":"225BD9F9-493D-F9E5-50A4-9BE0C59FB859",
    "token":"r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x4qhDd3J08HtHVj"
}
```

### 登出接口
**接口定义：**
- 请求终端：`客户端`
- 请求方式：`POST`
- 请求链接：`https://testapi.hcolda.com/api.php?type=application&commend=logout`

**请求参数列表：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|uuid|是|string|示例值:`225BD9F9-493D-F9E5-50A4-9BE0C59FB859`|
|token|是|string|示例值:`r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x...`|

**结果字段说明：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|state|是|string|示例值:`success`|

**原始数据：**
```json
{
    "state":"success"
}
```

### 获取密钥方式[客户端]
**接口定义：**
- 请求终端：`客户端`
- 请求方式：`POST`
- 请求链接：`https://testapi.hcolda.com/api.php?type=application&commend=aeskey`

**请求参数列表：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|uuid|是|string|示例值:`225BD9F9-493D-F9E5-50A4-9BE0C59FB859`|
|token|是|string|示例值:`r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x...`|

**结果字段说明：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|state|是|string|示例值:`success`|
|key|是|string|示例值:`cXFkd0E4TWtUT2twbEp3cU5ncTNQQTlaZ2lFRm4zU2Y=`|
|iv|是|string|示例值:`YUY1M2phMm45dUUyUlZnRw==`|

**原始数据：**
```json
{
    "state":"success",
    "key":"cXFkd0E4TWtUT2twbEp3cU5ncTNQQTlaZ2lFRm4zU2Y=\n",
    "iv":"YUY1M2phMm45dUUyUlZnRw==\n"
}
```

### 获取密钥方式[服务端]
**接口定义：**
- 请求终端：`服务端`
- 请求方式：`POST`
- 请求链接：`https://testapi.hcolda.com/api.php?type=server&commend=aeskey`

**请求参数列表：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|serverid|是|string|示例值:`A19686BE-6C5E-163B-B7B0-9F8C1CA2694B`|
|serverkey|是|string|示例值:`r7TE6Jh6qQOJc2tBTye1XrjTr3wFU4XjuyZhnla79SvMeReu3x...`|
|uuid|是|string|示例值:`225BD9F9-493D-F9E5-50A4-9BE0C59FB859`|

**结果字段说明：**

|字段名|必选|类型|说明|
|:---|:---|:---|:---|
|state|是|string|示例值:`success`|
|key|是|string|示例值:`cXFkd0E4TWtUT2twbEp3cU5ncTNQQTlaZ2lFRm4zU2Y=`|
|iv|是|string|示例值:`YUY1M2phMm45dUUyUlZnRw==`|

**原始数据：**
```json
{
    "state":"success",
    "key":"cXFkd0E4TWtUT2twbEp3cU5ncTNQQTlaZ2lFRm4zU2Y=\n",
    "iv":"YUY1M2phMm45dUUyUlZnRw==\n"
}
```
import { Sink, write_str, write_u8, write_u16, write_u32, read_str } from 'ts-binary';
import { DebugMessageType } from './message-type.enum';

export class DebugMessage {

  constructor(type: DebugMessageType = null, message: string = '', array: Uint8Array = null) {
    if (type) {
      if (array) {
        this.msgType = type;
        this.msg = DebugMessage.uint8ArrayToString(array);
      } else {
        this.fromArguments(type, message);
      }
    } else if (array) {
      this.parseHeader(array);
    }
  }
  header = '';
  version: number = null;
  msgType: DebugMessageType = null;
  padding_: number = null;
  msgLength: number = null;
  padding: number = null;
  msg: string;

  static stringToUint8Array(string: string): Uint8Array {
    const array: Uint8Array = new Uint8Array(string.length);
    for (let i = 0; i < string.length; i++) {
      array[i] = string.charCodeAt(i);
    }
    return array;
  }

  static uint8ArrayToString(array: Uint8Array): string {
    let string = '';
    for (const v of array) {
      string += String.fromCharCode(v);
    }
    return string;
  }

  fromArguments(type, message) {
    this.header = 'DMSG';
    this.version = 1;
    this.msgType = type;
    this.padding_ = 0;
    this.msgLength = message.length;
    this.padding = 0;
    this.msg = message;
  }

  parseHeader(array: Uint8Array) {
    if (array.length == 16) {
      console.log('Buffer: ', array.buffer);
      const sink = Sink(array.buffer);
      let msgheader = '';
      for (let i = 0; i < 4; i++) {
        msgheader += String.fromCharCode(sink.view.getUint8(i));
      }
      this.header = msgheader;
      this.version = sink.view.getUint8(4);
      console.log(sink.view.getUint8(5));
      this.msgType = sink.view.getUint8(5);
      this.padding_ = sink.view.getUint16(6);
      this.msgLength = sink.view.getUint32(8);
      this.padding = sink.view.getUint32(12);
      console.log('Generated Header: ', this);
    }
  }

  isCompleted(): boolean {
    return this.length() == this.msgLength + 16;
  }

  // toIntArray(): Uint8Array {
  //   let index = 0;
  //   const array: Uint8Array = new Uint8Array();
  //   for (let i = 0; i < this.header.length; i++) {
  //     console.log(this.header.charCodeAt(i));
  //     array.set([this.header.charCodeAt(i)], index);
  //     // array[index] = this.header.charCodeAt(i);
  //     index++;
  //   }
  //   // array[index] = this.version;
  //   // array[++index] = this.msgType;
  //   // array[++index] = this.padding_;
  //   // array[++index] = this.msgLength;
  //   // array[++index] = this.padding;
  //   // index++;
  //   array.set([this.version], index);
  //   array.set([this.msgType], ++index);
  //   array.set([this.padding_], ++index);
  //   array.set([this.msgLength], ++index);
  //   array.set([this.padding], ++index);
  //   index++;
  //   for (let i = 0; i < this.msg.length; i++) {
  //     array.set([this.msg.charCodeAt(i)], index);
  //     // array[index] = this.msg.charCodeAt(i);
  //     index++;
  //   }
  //   return array;
  // }

  toSink(): Sink {
    const buffer = new ArrayBuffer(16 + this.msgLength);
    const data: Sink = Sink(buffer);
    let index = 0;

    for (let i = 0; i < this.header.length; i++) {
      console.log(index);
      data.view.setUint8(index, this.header.charCodeAt(i));
      index++;
    }

    data.view.setUint8(index, this.version);
    index++;
    data.view.setUint8(index, this.msgType);
    index++;
    data.view.setUint16(index, this.padding_);
    index += 2;
    data.view.setUint32(index, this.msgLength);
    index += 4;
    data.view.setUint32(index, this.padding);
    index += 4;

    for (let i = 0; i < this.msg.length; i++) {
      data.view.setUint8(index, this.msg.charCodeAt(i));
      index++;
    }

    return data;
  }

  length(): number {
    return 16 + this.msg.length;
  }

  completedLength(): number {
    return 16 + this.msgLength - this.length();
  }

  missingLength(): number {
    return this.completedLength() - this.length();
  }

  appendMessage(buff: Uint8Array): Uint8Array {
    let returnBuff = buff;
    if (buff.length > this.missingLength()) {
      returnBuff = buff.subarray(this.missingLength());
    }
    else{
      returnBuff = new Uint8Array();
    }
    this.msg += DebugMessage.uint8ArrayToString(buff.subarray(0, this.missingLength()));
    return returnBuff;
  }

  headerIncomplete(): boolean {
    if (this.msgType == null
      || this.header.length == 0
      || this.version == null
      || this.padding_ == null
      || this.msgLength == null
      || this.padding == null) {
        return false;
      }
    return false;
  }
}

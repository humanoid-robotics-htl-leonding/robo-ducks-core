import {SerializeField, DataType} from 'ts-binary-serializer';

export class DebugMessageHeader {
  header = 'DMSG';
  version = 1;
  msgType = 100;
  padding_ = 0;
  msgLength = 4;
  padding = 0;
  msg = 'Yeet';

  toIntArray(): Uint8Array {
    let index = 0;
    const array: Uint8Array = new Uint8Array();
    for (let i = 0; i < this.header.length; i++) {
      array.set([this.header.charCodeAt(i)], index);
      // array[index] = this.header.charCodeAt(i);
      index++;
    }
    // array[index] = this.version;
    // array[++index] = this.msgType;
    // array[++index] = this.padding_;
    // array[++index] = this.msgLength;
    // array[++index] = this.padding;
    // index++;
    array.set([this.version], index);
    array.set([this.msgType], ++index);
    array.set([this.padding_], ++index);
    array.set([this.msgLength], ++index);
    array.set([this.padding], ++index);
    index++;
    for (let i = 0; i < this.msg.length; i++) {
      array.set([this.msg.charCodeAt(i)], index);
      // array[index] = this.msg.charCodeAt(i);
      index++;
    }
    return array;
  }
}

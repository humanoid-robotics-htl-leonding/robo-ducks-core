import { NaoElementComponent } from '../components/elements/nao-element/nao-element.component';

export interface NaoGridElement {
  id: number;
  type: string;
  rowStart: number;
  colStart: number;
  rowEnd: number;
  colEnd: number;
  component: NaoElementComponent;
}

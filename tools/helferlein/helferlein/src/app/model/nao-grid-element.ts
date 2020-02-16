import { NaoElementComponent } from '../components/elements/nao-element/nao-element.component';

export interface NaoGridElement {
  id: number,
  type: string,
  row: number;
  col: number;
  rowspan: number;
  colspan: number;
  component: NaoElementComponent;
}

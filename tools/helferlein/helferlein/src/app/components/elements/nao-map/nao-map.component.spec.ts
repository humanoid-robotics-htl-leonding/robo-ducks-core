import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoMapComponent } from './nao-map.component';

describe('NaoMapComponent', () => {
  let component: NaoMapComponent;
  let fixture: ComponentFixture<NaoMapComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoMapComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoMapComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});

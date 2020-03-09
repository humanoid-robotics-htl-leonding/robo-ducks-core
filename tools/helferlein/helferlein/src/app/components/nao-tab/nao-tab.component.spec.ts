import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoTabComponent } from './nao-tab.component';

describe('NaoTabComponent', () => {
  let component: NaoTabComponent;
  let fixture: ComponentFixture<NaoTabComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoTabComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoTabComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
